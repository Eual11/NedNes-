#include "../include/Ned2A03.h"
#include <cstdint>

// TODO: Fully implement sound
using namespace NedNes;

Ned2A03::Ned2A03() {
  // resetting stuff

  reset();
}

void Ned2A03::fillAudioBuffer(int16_t *audio_buffer, unsigned int size) {
  unsigned int read_count = 0x00;
  pulse1_osc.frequency = 440;
  pulse1_osc.dutycycle = 0.5;
  int H = 20;

  while (read_count < size) {
    double wave_a = 0;
    double wave_b = 0;
    for (int n = 1; n <= H; n++) {
      wave_a +=
          (sin(2.0 * pulse1_osc.pi * pulse1_osc.frequency * gTime * n)) / n;
      wave_b +=
          (sin(2.0 * pulse1_osc.pi *
               (pulse1_osc.frequency * gTime - pulse1_osc.dutycycle) * n)) /
          n;
    }
    int16_t wave = (wave_a - wave_b) * UINT16_MAX;

    audio_buffer[read_count] = UINT16_MAX * sin(gTime * 2 * 3.14 * 440);
    read_count += 1;
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

  gTime += (0.3333333333 / 1789773);
  // the apu cycle clocks once per other cpu clock which is 2x slower than the
  // cpu, which in turn is 3x slower than the ppu we clock the ppu and apu at
  // the same rate so we have to wait 6 cycles
  if (cycles % 6 == 0) {

    frame_counter++;
    pulse1_sequencer.clock(pulse1_enable, [](uint32_t &s) {
      s = ((s & 0x0001) << 7) | ((s & 0xFE) >> 1);
    });

    pulse1_osc.frequency =
        1789773.0 / (16.0 * (double)(pulse1_sequencer.reload + 1));
    pulse1_osc.amplitude = 1;
    pulse1_sample = (int16_t)(UINT16_MAX * pulse1_osc.sample(gTime));

    /* pulse1_sample = pulse1_sequencer.output; */
    /* audio_queue.push_back(pulse1_sample); */
    /* buffer.write(pulse1_sample); */
    current_sample_count++;
  }

  cycles++;
}
void Ned2A03::reset() {
  // TODO:
}
