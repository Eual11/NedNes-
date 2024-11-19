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
      pulse1.dutycycle = 0.125;
      break;
    }
    case 0x01: {
      pulse1.dutycycle = 0.250;
      break;
    }
    case 0x02: {
      pulse1.dutycycle = 0.50;
      break;
    }
    case 0x03: {
      pulse1.dutycycle = 0.75;
      break;
    }
    }

    pulse1_halt = data & 0x20;
    pulse1.lc_halt = data & 0x20;
  }
  if (addr == 0x4001) {
    // sweep
  }
  if (addr == 0x4002) {

    // timer low
    pulse1.reload = (pulse1.reload & 0xFF) | (data);
  }
  if (addr == 0x4003) {

    pulse1.length_counter = lc_lookup[(data >> 3) & 0x1F];
    // timer high
    pulse1.reload = ((uint16_t)(data & 0x7) << 8) | pulse1.reload & 0x00FF;
    pulse1.timer = pulse1.reload;
  }

  if (addr == 0x4015) {
    // status
    pulse1.enabled = data & 0x01;
    if (!pulse1.enabled)
      pulse1.length_counter = 0x00;
  }
  if (addr == 0x4017) {
    frame_counter_mode = (data & 0x40);
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

  // the apu cycle clocks once per other cpu clock which is 2x slower than the
  // cpu, which in turn is 3x slower than the ppu we clock the ppu and apu at
  // the same rate so we have to wait 6 cycles

  if (cycles % 6 == 0) {

    frame_counter++;

    if (!frame_counter_mode) {
      // 4 step mode

      bool quarter_frame = false;
      bool half_frame = false;

      if (frame_counter == 3729) {
        // clock envlope and triangle linear counter
      }
      if (frame_counter == 7458) {
        // clock both env, tri and lenfth counters, sweep units
        pulse1.clock_lc();
      }
      if (frame_counter == 11187) {

        // clock envlope and triangle linear counter
      }

      if (frame_counter >= 14915) {

        // clock both env, tri and lenfth counters, sweep units
        pulse1.clock_lc();
        // resetting frame counter
        frame_counter = 0;
      }
    } else {

      if (frame_counter == 3729) {
        // clock envlope and triangle linear counter
      }
      if (frame_counter == 7458) {
        // clock both env, tri and lenfth counters, sweep units
        pulse1.clock_lc();
      }
      if (frame_counter == 11187) {

        // clock envlope and triangle linear counter
      }

      if (frame_counter >= 18645) {

        // clock both env, tri and lenfth counters, sweep units
        pulse1.clock_lc();
        // resetting frame counter
        frame_counter = 0;
      }
    }

    // check if Audio queue has some room for more samples
    int samples_in_queue = SDL_GetQueuedAudioSize(device);

    if (samples_in_queue < audio_queue_threshould) {

      double pulse1_sample = 0.0f;

      accumulator += sample_per_apu_cycle;

      std::vector<int16_t> new_samples;
      while (accumulator >= 1.0f || new_samples.size() < 512) {
        // we generated atleast once sample

        accumulator -= 1.0f;
        pulse1.frequency = 1789773.0 / (16.0 * (double)(pulse1.timer + 1));

        // NOTE: debug

        double frequency_step = 2 * M_PI * (pulse1.frequency / 44100.0);
        double t = phase_accumulator / (2.0 * M_PI * pulse1.frequency);
        phase_accumulator += frequency_step;

        // resetting phase
        if (phase_accumulator >= 2 * M_PI)
          phase_accumulator -= 2 * M_PI;

        pulse1_sample = (int16_t)(INT16_MAX * pulse1.sample(t));

        new_samples.push_back(pulse1_sample);

        double mixed_sample = pulse1_sample;

        // used for debug purpose
        audioData.push_back(pulse1_sample);
      }

      SDL_QueueAudio(device, new_samples.data(),
                     new_samples.size() * sizeof(int16_t));

      current_sample_count++;
    }
  }

  cycles++;
}
void Ned2A03::reset() {
  // TODO:
}
