#ifndef __NED2AO3__
#define __NED2AO3__

#include <SDL2/SDL_audio.h>
#include <cstdint>
#include <deque>
#include <fstream>
#include <functional>
#include <iostream>
#include <ostream>
#include <string>
#include <vector>
struct WAVHeader {
  char riff[4] = {'R', 'I', 'F', 'F'};
  uint32_t fileSize;
  char wave[4] = {'W', 'A', 'V', 'E'};
  char fmt[4] = {'f', 'm', 't', ' '};
  uint32_t fmtLength = 16;
  uint16_t audioFormat = 1; // PCM
  uint16_t numChannels = 1; // Mono
  uint32_t sampleRate = 44100;
  uint32_t byteRate;
  uint16_t blockAlign;
  uint16_t bitsPerSample = 16;
  char data[4] = {'d', 'a', 't', 'a'};
  uint32_t dataSize;
};

namespace NedNes {

struct RingBuffer {
public:
  RingBuffer(unsigned int size = 2 << 16) {
    buffer.resize(size);
    N = size;
  }
  uint32_t read(unsigned int idx) {
    // if rempty read null, in case of our APU
    // pure silence
    if (read_index == write_index)
      return 0x00;
    unsigned int data = buffer[read_index];
    // once samples has been read, we no longer need it
    buffer[read_index] = 0x00;
    read_index = (read_index + 1) % N;

    return data;
  }
  void write(uint16_t val) {
    if ((write_index + 1) % N == read_index)
      return;

    buffer[(write_index) % N] = val;
    write_index = (write_index + 1) % N;
  }

  // this function should be called before any read and write operation if it's
  // needed
  void resize(unsigned int new_size) { buffer.resize(new_size); }

private:
  std::vector<int16_t> buffer;
  unsigned int read_index = 0;
  unsigned int write_index = 0;
  unsigned int N = 0x00;
};
class Ned2A03 {

public:
  int last_sample_count = 0x00;
  std::vector<int16_t> audioData;
  int current_sample_count = 0x00;
  void writeWAV(const std::string &filename) {
    WAVHeader header;
    header.sampleRate = 44100;
    header.numChannels = 1;    // Mono
    header.bitsPerSample = 16; // 16-bit samples
    header.byteRate = 44100 * header.numChannels * header.bitsPerSample / 8;
    header.blockAlign = header.numChannels * header.bitsPerSample / 8;
    header.dataSize = audioData.size() * sizeof(int16_t);
    header.fileSize = 36 + header.dataSize;

    std::ofstream file(filename, std::ios::binary);
    if (!file) {
      std::cerr << "Failed to open file for writing\n";
      return;
    }

    // Write WAV header
    file.write((const char *)&header, sizeof(header));

    // Write audio data
    file.write((const char *)audioData.data(), header.dataSize);

    file.close();
  }
  Ned2A03(SDL_AudioDeviceID id);
  ~Ned2A03() = default;
  uint8_t cpuRead(uint16_t addr);
  void cpuWrite(uint16_t addr, uint8_t data);
  void clock();
  void reset();
  float gTime = 0.0f;
  int16_t pulse1_sample;
  SDL_AudioDeviceID device = 0;
  int audio_queue_threshould = 4096 * 4;

  struct oscpulse {
    double frequency = 0;
    double dutycycle = 0;
    double amplitude = 1;
    double pi = 3.14159;
    double harmonics = 20;

    double sample(double t) {
      double a = 0;
      double b = 0;
      double p = dutycycle * 2.0 * pi;

      auto approxsin = [](float t) {
        float j = t * 0.15915;
        j = j - (int)j;
        return 20.785 * j * (j - 0.5) * (j - 1.0f);
      };

      for (double n = 1; n < harmonics; n++) {
        double c = n * frequency * 2.0 * pi * t;
        a += -approxsin(c) / n;
        b += -approxsin(c - p * n) / n;
      }

      return (2.0 * amplitude / pi) * (a - b);
    }
  };
  struct sequencer {

    uint32_t sequence = 0x00000000;
    uint16_t timer = 0x00000000;
    uint16_t reload = 0x00000000;
    uint32_t new_sequence = 0x00000000;
    uint16_t output = 0x00;

    uint16_t clock(bool enable, std::function<void(uint32_t &)> func) {

      if (enable) {
        timer--;
        if (timer == 0xFFFF) {
          timer = reload;
          func(sequence);
          output = sequence & 0x01;
        }
      }
      return output;
    }
  };
  void fillAudioBuffer(int16_t *buffer, unsigned int size);

private:
  std::deque<int16_t> audio_queue;
  uint32_t frame_counter = 0;
  uint32_t cycles = 0;
  float pulse1_dutycycle = 0.0f;
  bool pulse1_halt = false;
  bool pulse1_enable = false;
  sequencer pulse1_sequencer;
  oscpulse pulse1_osc;
  RingBuffer buffer;
};
} // namespace NedNes

#endif
