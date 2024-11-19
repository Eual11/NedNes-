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
  int audio_queue_threshould = 4096 * 2;

  struct PulseChannel {
    uint32_t timer = 0.0;
    uint32_t reload = 0;
    double frequency = 0.0f;
    double dutycycle = 0.0;

    // enable and length counter
    bool enabled = false;
    uint32_t length_counter = 0;
    bool lc_halt = false;
    bool sweep_mute = false;

    // envelope parameters

    bool constant_volume = false;
    bool envlope_loop = false;

    uint8_t envlope_period = 0x00;

    uint8_t envlope_divider = 0x00;
    uint8_t envlope_counter = 0x00;

    bool start_envlope_flag = false;

    // sweep parameters

    bool sweep_enable = false;
    bool sweep_negate = false;
    bool sweep_reload = false;
    uint8_t sweep_divider = 0x00;
    uint8_t sweep_period = 0x00;
    uint8_t sweep_shift = 0x00;

    double harmonics = 20;
    double volume = 1.0f;

    double getVolume() const {
      return ((constant_volume)) ? envlope_period : envlope_counter;
    }
    double sample(double t) {
      if ((length_counter == 0 && (!lc_halt)) || !enabled) {
        return 0.0f;
      }
      if (sweep_mute)
        return 0x00;
      double b = 0;
      double a = 0;
      double p = dutycycle * 2.0 * M_PI;

      for (double n = 1; n < harmonics; n++) {
        double c = frequency * n * 2.0 * M_PI * t;
        a += -sin(c) / n;
        b += -sin(c - p * n) / n;
      }

      return ((getVolume() / 15.0f) / (2.0 * M_PI)) * (a - b);
    }

    void clock_lc() {
      if (!enabled)
        return;
      if (length_counter > 0 && !lc_halt)
        --length_counter;
    }
    void clock_envlope() {
      if (!enabled)
        return;

      if (start_envlope_flag) {
        start_envlope_flag = false;
        envlope_counter = 15;
        envlope_divider = envlope_period;
      } else {
        if (envlope_divider > 0) {
          envlope_divider--;
        } else {
          envlope_divider = envlope_period;

          if (envlope_counter > 0) {
            // reloading divider
            envlope_counter--;
          } else {
            if (envlope_loop) {
              envlope_counter = 15;
            }
          }
        }
      }
    }

    void clock_sweep(uint8_t channel_id) {
      if (!sweep_enable || !enabled || timer < 8 || timer > 0x07FF) {
        return; // Sweep is inactive or out of range
      }

      // If divider reaches 0, perform sweep
      if (sweep_divider == 0) {
        // Calculate change
        uint16_t change = timer >> sweep_shift;

        if (sweep_negate) {
          timer -= change;
          if (channel_id == 1) {
            timer--; // Extra subtract for Pulse 1
          }
        } else {
          timer += change;
        }

        // Timer overflow check
        if (timer > 0x07FF || timer < 8) {
          sweep_mute = true;
        }

        // Reset divider
        sweep_divider = sweep_period;
      } else {
        sweep_divider--;
      }

      // Reload divider if reload flag is set
      if (sweep_reload) {
        sweep_divider = sweep_period;
        sweep_reload = false;
      }
    }
  };
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
        double c = frequency * n * 2.0 * pi * t;
        a += -sin(c) / n;
        b += -sin(c - p * n) / n;
      }

      return (amplitude / (2.0 * pi)) * (a - b);
    }
  };

  void fillAudioBuffer(int16_t *buffer, unsigned int size);

private:
  std::deque<int16_t> audio_queue;
  uint32_t frame_counter = 0;
  bool frame_counter_mode = 0; // 0 for 4 step mode, 1 for 5 step mode
  uint32_t cycles = 0;
  const uint8_t lc_lookup[32] = {10, 254, 20,  2,  40, 4,  80, 6,  160, 8,  60,
                                 10, 14,  12,  26, 14, 12, 16, 24, 18,  48, 20,
                                 96, 22,  192, 24, 72, 26, 16, 28, 32,  30};

  double accumulator = 0.0f;
  double phase_accumulator1 = 0.0f;
  double phase_accumulator2 = 0.0f;
  double prev_sample = 0.0f;
  float pulse1_dutycycle = 0.0f;
  bool pulse1_halt = false;
  bool pulse1_enable = false;
  PulseChannel pulse1;
  PulseChannel pulse2;
};
} // namespace NedNes

#endif
