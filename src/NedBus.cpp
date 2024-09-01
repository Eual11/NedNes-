#include "../include/NedBus.h"
#define _CRT_SECURE_NO_WARNINGS
#include <cstdint>
#include <memory>

// NedBus Constructor, currently only just filling the ram with 0s
NedNes::NedBus::NedBus() { std::fill(std::begin(ram), std::end(ram), 0x00); }

// Reading from memory, the NedBus is 16 bit wide bus so it is capable of
// addressing 64Kb of memory

uint8_t NedNes::NedBus::cpuRead(uint16_t addr) {

  uint8_t data = 0x00;
  if (cart->cpuRead(addr, data)) {
  } else if (addr >= 0x0000 && addr <= 0x1FFF) {
    // mirroring the cpu read
    data = ram[addr & 0x7FF];
  } else if (addr >= 0x2000 && addr <= 0x3FFF) {
    data = ppu->cpuRead(addr & 0x2007);
  }

  return data;
}
// Writing to the bus, currently we only have the ram so we write to it
void NedNes::NedBus::cpuWrite(uint16_t addr, uint8_t val) {

  if (cart->cpuWrite(addr, val)) {
  } else if (addr >= 0x0000 && addr <= 0x1FFF) {
    // mirroring the cpu write
    ram[addr & 0x7FF] = val;
  } else if (addr >= 0x2000 && addr <= 0x3FFF) {
    ppu->cpuWrite(addr & 0x2007, val);
  }
}

// connecting a cpu to the bus
void NedNes::NedBus::connectCpu(std::shared_ptr<Ned6502> _cpu) { cpu = _cpu; }
void NedNes::NedBus::connectPpu(std::shared_ptr<Ned2C02> _ppu) { ppu = _ppu; }
void NedNes::NedBus::connectCartridge(std::shared_ptr<NedCartrdige> _cart) {

  this->cart = _cart;
}
void NedNes::NedBus::clock() {

  ppu->clock();
  if (SystemClock % 3 == 0) {
    cpu->clock();
  }
  if (ppu->nmi) {
    /* printf("Non Maskable intrupt\n"); */
    ppu->PPUSTATUS.bits.vblank = 0x00; // clearing vblank
    ppu->nmi = false;
    cpu->nmi();
  }
  SystemClock++;
}
