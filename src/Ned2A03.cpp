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
    // DDLC VVVV
    //
    pulse1.envlope_loop = data & 0x20;
    pulse1.envlope_period = data & 0xF;
    pulse1.constant_volume = (data & 0x10) != 0;
    pulse1.lc_halt = data & 0x20;
  }
  if (addr == 0x4001) {
    // pulse1 sweep
    pulse1.sweep_enable = data & 0x80;
    pulse1.sweep_period = (data & 0x70) >> 4;
    pulse1.sweep_negate = (data & 0x8);
    pulse1.sweep_shift = (data & 0x7);
    pulse1.sweep_reload = true;
  }
  if (addr == 0x4002) {

    // pulse1 timer low
    pulse1.reload = (pulse1.reload & 0xFF00) | (data);
  }
  if (addr == 0x4003) {

    if (pulse1.enabled)
      pulse1.length_counter = lc_lookup[(data >> 3) & 0x1F];
    // timer high
    pulse1.reload = ((uint16_t)(data & 0x7) << 8) | (pulse1.reload & 0x00FF);
    pulse1.timer = pulse1.reload;
    pulse1.start_envlope_flag = true;
  }

  if (addr == 0x4004) {
    switch ((data & 0xC0) >> 6) {

    case 0x00: {
      pulse2.dutycycle = 0.125;
      break;
    }
    case 0x01: {
      pulse2.dutycycle = 0.250;
      break;
    }
    case 0x02: {
      pulse2.dutycycle = 0.50;
      break;
    }
    case 0x03: {
      pulse2.dutycycle = 0.75;
      break;
    }
    }
  }
  if (addr == 0x4005) {
    // pulse2 sweep
    pulse2.sweep_enable = data & 0x80;
    pulse2.sweep_period = (data & 0x70) >> 4;
    pulse2.sweep_negate = (data & 0x8);
    pulse2.sweep_shift = (data & 0x7);
    pulse2.sweep_reload = true;
  }

  if (addr == 0x4006) {

    // pulse2 timer low
    pulse2.reload = (pulse2.reload & 0xFF00) | (data);
  }

  if (addr == 0x4007) {

    if (pulse2.enabled)
      pulse2.length_counter = lc_lookup[(data >> 3) & 0x1F];
    // timer high
    pulse2.reload = ((uint16_t)(data & 0x7) << 8) | (pulse2.reload & 0x00FF);
    pulse2.timer = pulse2.reload;
    pulse2.start_envlope_flag = true;
  }
  if (addr == 0x4015) {
    // status
    bool pulse1_new_enabled = (data & 0x01) != 0;
    bool pulse2_new_enabled = (data & 0x02) != 0;

    if (pulse1.enabled && !pulse1_new_enabled) {
      pulse1.length_counter = 0x00;
    }

    if (pulse2.enabled && !pulse2_new_enabled) {
      pulse2.length_counter = 0x00;
    }
    pulse1.enabled = pulse1_new_enabled;
    pulse2.enabled = pulse2_new_enabled;
  }
  if (addr == 0x4017) {
    frame_counter_mode = (data & 0x40) != 0;
  }
}
uint8_t Ned2A03::cpuRead(uint16_t addr) {
  uint8_t data = 0x00;

  return data;
}

void Ned2A03::clock() {

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

        pulse1.clock_envlope();
        pulse2.clock_envlope();
      }
      if (frame_counter == 7457) {
        // clock both env, tri and lenfth counters, sweep units
        pulse1.clock_envlope();
        pulse1.clock_lc();
        pulse1.clock_sweep(1);

        // pulse2
        pulse2.clock_envlope();
        pulse2.clock_lc();
        pulse2.clock_sweep(2);
      }
      if (frame_counter == 11186) {

        // clock envlope and triangle linear counter
        pulse1.clock_envlope();

        pulse2.clock_envlope();
      }

      if (frame_counter >= 14916) {

        // clock both env, tri and lenfth counters, sweep units
        pulse1.clock_envlope();
        pulse1.clock_lc();
        pulse1.clock_sweep(1);

        // pulse2

        pulse2.clock_envlope();
        pulse2.clock_lc();
        pulse2.clock_sweep(2);
        //  resetting frame counter
        frame_counter = 0;
      }
    } else {

      if (frame_counter == 3729) {
        // clock envlope and triangle linear counter
        pulse1.clock_envlope();
        pulse2.clock_envlope();
      }
      if (frame_counter == 7457) {
        // clock both env, tri and lenfth counters, sweep units
        pulse1.clock_lc();
        pulse1.clock_envlope();
        pulse1.clock_sweep(1);

        // pulse2

        pulse2.clock_lc();
        pulse2.clock_envlope();
        pulse2.clock_sweep(2);
      }
      if (frame_counter == 11186) {

        // clock envlope and triangle linear counter
        pulse1.clock_envlope();

        pulse2.clock_envlope();
      }

      if (frame_counter >= 18645) {

        // clock both env, tri and lenfth counters, sweep units
        pulse1.clock_envlope();
        pulse1.clock_lc();
        pulse1.clock_sweep(1);

        // pulse 2
        pulse2.clock_envlope();
        pulse2.clock_lc();
        pulse2.clock_sweep(2);

        //  resetting frame counter
        frame_counter = 0;
      }
    }

    // check if Audio queue has some room for more samples
    int samples_in_queue = SDL_GetQueuedAudioSize(device);

    if (samples_in_queue < audio_queue_threshould) {

      int16_t pulse1_sample = 0;
      int16_t pulse2_sample = 0;

      int16_t mixed_sample = 0x00;
      accumulator += sample_per_apu_cycle;

      std::vector<int16_t> new_samples;
      while (accumulator >= 1.0f || new_samples.size() < 512) {
        // we generated atleast once sample

        gTime += (1.0 / 44100.0);
        accumulator -= 1.0f;
        pulse1.frequency = 1789773.0 / (16.0 * (double)(pulse1.timer + 1.0));
        pulse2.frequency = 1789773.0 / (16.0 * (double)(pulse2.timer + 1.0));

        double frequency_step1 = 2 * M_PI * (pulse1.frequency / 44100.0);
        double frequency_step2 = 2 * M_PI * (pulse2.frequency / 44100.0);
        double t1 = phase_accumulator1 / (2.0 * M_PI * pulse1.frequency);
        double t2 = phase_accumulator2 / (2.0 * M_PI * pulse2.frequency);

        phase_accumulator1 += frequency_step1;
        phase_accumulator2 += frequency_step2;

        // resetting phase
        if (phase_accumulator1 >= 2 * M_PI)
          phase_accumulator1 -= 2 * M_PI;
        if (phase_accumulator2 >= 2 * M_PI)
          phase_accumulator2 -= 2 * M_PI;

        pulse1_sample = (int16_t)(INT16_MAX * pulse1.sample(t1));
        pulse2_sample = (int16_t)(INT16_MAX * pulse2.sample(t2));

        double mixed_sample = pulse1_sample * 0.5 + pulse2_sample * 0.5;

        new_samples.push_back(mixed_sample);

        // used for debug purpose
        audioData.push_back(mixed_sample);
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
