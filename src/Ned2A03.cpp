#include "../include/Ned2A03.h"
#include <SDL2/SDL_stdinc.h>
#include <cstdint>

// TODO: Fully implement sound
using namespace NedNes;

Ned2A03::Ned2A03(SDL_AudioDeviceID id) {
  // resetting stuff

  device = id;
  reset();
}

void Ned2A03::fillAudioBuffer(int16_t *audio_buffer, unsigned int size) {

  int read_count = 0;

  while (audio_queue.size() != 0 && read_count < size) {

    audio_buffer[read_count] = audio_queue.front();
    read_count++;
    audio_queue.pop_front();
  }
}

void Ned2A03::cpuWrite(uint16_t addr, uint8_t data) {

  // pulse 1
  // duty, envelope, lc, halt
  if (addr == 0x4000) {
    switch ((data & 0xC0) >> 6) {

    case 0x00: {
      pulse1_sequencer.new_sequence = 0b00000001;
      pulse1_osc.dutycycle = 0.125;
      break;
    }
    case 0x01: {
      pulse1_sequencer.new_sequence = 0b01100000;
      pulse1_osc.dutycycle = 0.250;
      break;
    }
    case 0x02: {
      pulse1_sequencer.new_sequence = 0b011110000;
      pulse1_osc.dutycycle = 0.50;
      break;
    }
    case 0x03: {
      pulse1_sequencer.new_sequence = 0b010011111;
      pulse1_osc.dutycycle = 0.75;
      break;
    }
    }

    pulse1_halt = data & 0x20;
    pulse1_sequencer.sequence = pulse1_sequencer.new_sequence;
  }
  if (addr == 0x4001) {
    // sweep
  }
  if (addr == 0x4002) {

    // timer low
    pulse1_sequencer.reload = (pulse1_sequencer.reload & 0xFF00) | data;
  }
  if (addr == 0x4003) {
    // timer high

    pulse1_sequencer.reload =
        ((uint16_t)(data & 0x7) << 8) | pulse1_sequencer.reload & 0x00FF;
    pulse1_sequencer.timer = pulse1_sequencer.reload;
    pulse1_sequencer.sequence = pulse1_sequencer.new_sequence;
  }

  if (addr == 0x4015) {
    // status
    pulse1_enable = data & 0x01;
  }
}
uint8_t Ned2A03::cpuRead(uint16_t addr) {
  uint8_t data = 0x00;

  return data;
}

void Ned2A03::clock() {

  static double accumulator = 0.0f;
  static double prev_sample = 0.0f;
  static double phase_accumulator = 0.0f;

  double sample_per_apu_cycle = 44100.0 / 1789773.0;

  gTime += (0.3333333333 / 1789773);
  // the apu cycle clocks once per other cpu clock which is 2x slower than the
  // cpu, which in turn is 3x slower than the ppu we clock the ppu and apu at
  // the same rate so we have to wait 6 cycles

  if (cycles % 6 == 0) {
    int samples_in_queue = SDL_GetQueuedAudioSize(device);

    if (samples_in_queue < audio_queue_threshould) {

      double amp = 0.5;
      accumulator += sample_per_apu_cycle;

      std::vector<int16_t> new_samples;
      while (accumulator >= 1.0f || new_samples.size() < 512) {
        // we generated atleast once sample
        accumulator -= 1.0f;

        pulse1_osc.frequency =
            1789773.0 / (16.0 * (double)(pulse1_sequencer.reload + 1));

        double frequency_step = 2 * M_PI * pulse1_osc.frequency / 44100.0;

        double t = phase_accumulator / (2.0 * M_PI * pulse1_osc.frequency);

        double sin_sample = pulse1_osc.sample(t);
        /* sin_sample = sin(2 * M_PI * pulse1_osc.frequency * t); */
        phase_accumulator += frequency_step;
        if (phase_accumulator >= 2 * M_PI)
          phase_accumulator -= 2 * M_PI;

        /* sin_sample = sin(phase_accumulator); */
        pulse1_sample = (int16_t)(INT16_MAX * sin_sample);

        new_samples.push_back(pulse1_sample);
        audioData.push_back(pulse1_sample);
      }

      SDL_QueueAudio(device, new_samples.data(),
                     new_samples.size() * sizeof(int16_t));

      frame_counter++;
      current_sample_count++;
    }
  }

  cycles++;
}
void Ned2A03::reset() {
  // TODO:
}
