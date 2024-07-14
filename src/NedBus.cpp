#include "../include/NedBus.h"
#include <stdint.h>

// NedBus Constructor, currently only just filling the ram with 0s
NedNes::NedBus::NedBus() { std::fill(std::begin(ram), std::end(ram), 0x00); }

// Reading from memory, the NedBus is 16 bit wide bus so it is capable of
// addressing 64Kb of memory

uint8_t NedNes::NedBus::read(uint16_t addr) {
  if (addr >= 0x0000 && addr <= 0xFFFF) {
    return ram[addr];
  }
  return 0x00;
}
// Writing to the bus, currently we only have the ram so we write to it
void NedNes::NedBus::write(uint16_t addr, uint8_t val) {
  if (addr >= 0x0000 && addr <= 0xFFFF) {
    ram[addr] = val;
  }
}
// connecting a cpu to the bus
void NedNes::NedBus::ConnectCpu(NedNes::Ned6502 *_cpu) { cpu = _cpu; }
