#include "../include/NedBus.h"
#include <stdint.h>

// NedBus Constructor, currently only just filling the ram with 0s
NedNes::NedBus::NedBus() { std::fill(std::begin(ram), std::end(ram), 0x00); }

// Reading from memory, the NedBus is 16 bit wide bus so it is capable of
// addressing 64Kb of memory

uint8_t NedNes::NedBus::cpuRead(uint16_t addr) {
  if (addr >= 0x0000 && addr <= 0x1FFF) {
    // mirroring the cpu read
    return ram[addr & 0x7FF];
  }
  if (addr >= 0x2000 && addr <= 0x3FFF) {
    return ppu->cpuRead(addr & 0x2000);
  }
  return 0x00;
}
// Writing to the bus, currently we only have the ram so we write to it
void NedNes::NedBus::cpuWrite(uint16_t addr, uint8_t val) {
  if (addr >= 0x0000 && addr <= 0x1FFF) {
    // mirroring the cpu write
    ram[addr & 0x7FF] = val;
  }
  if (addr >= 0x2000 && addr <= 0x3FFF) {
    ppu->cpuWrite(addr & 0x2000, val);
  }
}
// connecting a cpu to the bus
void NedNes::NedBus::connectCpu(NedNes::Ned6502 *_cpu) { cpu = _cpu; }

void NedNes::NedBus::connectPpu(std::shared_ptr<Ned2C02> _ppu) { ppu = _ppu; }
