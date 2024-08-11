#include "../include/Ned2CO2.h"
#define _CRT_SECURE_NO_WARNINGS
#include <memory>

NedNes::Ned2C02::Ned2C02() {
  // TODO: load the palette tables
  //
  cycles = 0;
  scanlines = 0;
}

uint8_t NedNes::Ned2C02::cpuRead(uint16_t addr) {
  switch (addr) {
  case 0x00: {
    // PPU Control Register
    break;
  }

  case 0x01: {
    // PPU Mask Registers
    break;
  }

  case 0x02: {
    // PPU Status Register
    break;
  }

  case 0x03: {
    // OAM Address Register
    break;
  }
  case 0x04: {
    // OAM Data Register
    break;
  }
  case 0x05: {
    // PPU Scroll register
    break;
  }
  case 0x06: {
    // PPU Address Register
    break;
  }
  case 0x07: {
    // PPU Data Register
  }
  }

  return 0x00;
}
void NedNes::Ned2C02::cpuWrite(uint16_t addr, uint8_t data) {
  switch (addr) {
  case 0x00: {
    // PPU Control Register
    break;
  }

  case 0x01: {
    // PPU Mask Registers
    break;
  }

  case 0x02: {
    // PPU Status Register
    break;
  }

  case 0x03: {
    // OAM Address Register
    break;
  }
  case 0x04: {
    // OAM Data Register
    break;
  }
  case 0x05: {
    // PPU Scroll register
    break;
  }
  case 0x06: {
    // PPU Address Register
    break;
  }
  case 0x07: {
    // PPU Data Register
  }
  }
}

uint8_t NedNes::Ned2C02::ppuRead(uint8_t addr) {

  uint8_t data = 0x00;

  // do something idk yet lmao
  //
  return data;
}
void NedNes::Ned2C02::ppuWrite(uint16_t addr, uint8_t data) {

  // do something idk yet lmao
}

void NedNes::Ned2C02::connectBus(std::shared_ptr<NedBus> _bus) { bus = _bus; }
void NedNes::Ned2C02::connectCart(std::shared_ptr<NedCartrdige> _cart) {
  cart = _cart;
}

void NedNes::Ned2C02::clock() {
  // I don't know what to do lmao
}
