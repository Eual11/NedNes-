#include <cstdio>
#define _CRT_SECURE_NO_WARNINGS
#include "../include/Ned2CO2.h"
#include <cstdint>
#include <memory>
// printf("PPUADDR: %04X, addr_latch: %d, PPUSTATUS: %02X\n", PPUADDR,
// addr_latch, PPUSTATUS.value);
NedNes::Ned2C02::Ned2C02(SDL_Renderer *gRenderer) {

  cycles = 0;
  scanlines = 0;
  screenTexture = SDL_CreateTexture(gRenderer, SDL_PIXELFORMAT_RGBA8888,
                                    SDL_TEXTUREACCESS_STREAMING, 256, 240);
  patternTableTexture[0] =
      SDL_CreateTexture(gRenderer, SDL_PIXELFORMAT_RGBA8888,
                        SDL_TEXTUREACCESS_STREAMING, 128, 128);
  patternTableTexture[1] =
      SDL_CreateTexture(gRenderer, SDL_PIXELFORMAT_RGBA8888,
                        SDL_TEXTUREACCESS_STREAMING, 128, 128);
}
NedNes::Ned2C02::~Ned2C02() {
  printf("TEXTURES Destroyed SUCESSFULLY\n");
  SDL_DestroyTexture(screenTexture);
  SDL_DestroyTexture(patternTableTexture[0]);
  SDL_DestroyTexture(patternTableTexture[1]);
  printf("TEXTURES Destroyed SUCESSFULLY\n");
}

uint8_t NedNes::Ned2C02::cpuRead(uint16_t addr) {
  uint8_t data = 0x00;
  switch (addr & 0xF) {
  case 0x00: {
    // PPU Control Register

    data = PPUCTRL.value;
    break;
  }

  case 0x01: {
    // PPU Mask Registers
    data = PPUMASK.value;
    break;
  }

  case 0x02: {
    // PPU Status Register
    PPUSTATUS.bits.vblank = 1;
    data = (PPUSTATUS.value & 0xE0) | (buffered_data & 0x1F);

    // reseting vblank
    PPUSTATUS.bits.vblank = 0;
    // reseting PPUADDR latch
    addr_latch = false;
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

    data = buffered_data;
    buffered_data = ppuRead(PPUADDR);

    if (addr >= 0x3F00) {
      // palette table doesn't rely on buffering
      data = buffered_data;
    }
    if (PPUCTRL.bits.increment_mode == 0) {
      PPUADDR += 1;
    } else {
      PPUADDR += 32;
    }
    PPUADDR &= 0x3FFF; // mirroring PPUADDR
    break;
  }
  }
  return data;
}
void NedNes::Ned2C02::cpuWrite(uint16_t addr, uint8_t data) {
  switch (addr & 0xF) {
  case 0x00: {
    // PPU Control Register
    break;
    PPUCTRL.value = data;
  }

  case 0x01: {
    // PPU Mask Registers
    PPUMASK.value = data;
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
    if (!addr_latch) {
      // reading hi byte
      PPUADDR = data << 8;
    } else {
      PPUADDR |= data;
    }
    addr_latch = !addr_latch; // flipping address latch
    break;
  }
  case 0x07: {
    // PPU Data Register

    // writing data to the address in ppu addr
    ppuWrite(PPUADDR, data);

    // increaming ppu addr based on
    if (PPUCTRL.bits.increment_mode == 0) {
      // going across
      PPUADDR += 1;
    } else {
      // going downward
      PPUADDR += 32;
    }
    PPUADDR &= 0x3FFF; // mirroring PPUADDR
    break;
  }
  }
}

uint8_t NedNes::Ned2C02::ppuRead(uint16_t addr) {

  uint8_t data = 0x00;

  if (cart->ppuRead(addr, data)) {
  } else if (addr >= 0x0000 && addr <= 0x1FFF) {
    // read from pattern table

    data = patternTable[(addr & 0x1000) >> 12][addr & 0xFFF];
  } else if (addr >= 0x2000 && addr <= 0x2EFF) {
    // nametable stuff
    // TODO: implement writing to nametable
  } else if (addr >= 0x3000 && addr <= 0x3FFF) {
    addr &= 0x1F;
    if (addr == 0x0010)
      addr = 0x0000;
    if (addr == 0x0014)
      addr = 0x0004;
    if (addr == 0x0018)
      addr = 0x0008;
    if (addr == 0x001C)
      addr = 0x000C;
    return paletteTable[addr];
  }

  // do something idk yet lmao
  //
  return data;
}
void NedNes::Ned2C02::ppuWrite(uint16_t addr, uint8_t data) {

  if (cart->ppuWrite(addr, data)) {
    // cartridge having veto power

  } else if (addr >= 0x0000 && addr <= 0x1FFF) {
    // write to pattern table
    patternTable[(addr & 0x1000) >> 12][addr & 0xFFF] = data;
  } else if (addr >= 0x2000 && addr <= 0x2EFF) {
    // TODO: implement writing to nametable
  } else if (addr >= 0x3000 && addr <= 0x3FFF) {

    addr &= 0x1F;
    // hardincoding the mirroring
    if (addr == 0x0010)
      addr = 0x000;
    if (addr == 0x0014)
      addr = 0x0004;
    if (addr == 0x0018)
      addr = 0x0008;
    if (addr == 0x001C)
      addr = 0x000C;
    paletteTable[addr] = data;
  }
  // do something idk yet lmao
}

void NedNes::Ned2C02::connectBus(std::shared_ptr<NedBus> _bus) { bus = _bus; }
void NedNes::Ned2C02::connectCart(std::shared_ptr<NedCartrdige> _cart) {
  cart = _cart;
}

void NedNes::Ned2C02::clock() {

  // advancing the clock count

  cycles++;
  frameComplete = false;
  if (cycles >= 341) {
    cycles = 0;
    scanlines++;
  }

  if (scanlines >= 0 && scanlines <= 239) {
    // visible scan lines
  } else if (scanlines >= 240 && scanlines <= 260) {
    if (cycles == 1) {
      PPUSTATUS.bits.vblank = 0x1;
    }
  } else if (scanlines >= 261) {
    if (cycles == 1) {
      scanlines = -1;
      frameComplete = true;
      PPUSTATUS.bits.vblank = 0x00;
    }
  }
}

SDL_Texture *NedNes::Ned2C02::getScreenTexture() { return screenTexture; }
SDL_Texture *NedNes::Ned2C02::getPatternTable(uint8_t i, uint8_t palette) {

  Uint32 *pixels;
  int pitch;
  SDL_Texture *tex = patternTableTexture[i];

  SDL_LockTexture(tex, nullptr, (void **)(&pixels), &pitch);
  for (int yTile = 0; yTile < 16; yTile++) {
    for (int xTile = 0; xTile < 16; xTile++) {

      uint16_t offset = yTile * 256 + 16 * xTile;

      for (int row = 0; row < 8; row++) {
        uint8_t lsb_pixel = ppuRead(0x1000 * i + offset + row);
        uint8_t msb_pixel = ppuRead(0x1000 * i + offset + row + 8);

        for (int col = 0; col < 8; col++) {
          uint8_t pixel = (lsb_pixel & 0x01) | ((msb_pixel & 0x01) << 1);

          lsb_pixel >>= 1;
          msb_pixel >>= 1;

          int yPos = yTile * 8 + row;
          int xPos = xTile * 8 + (7 - col);
          SET_PIXEL(pixels, xPos, yPos, pitch,
                    getColorFromPalette(palette, pixel));
        }
      }
    }
  }

  SDL_UnlockTexture(tex);
  return tex;
}
Uint32 NedNes::Ned2C02::getColorFromPalette(uint8_t palette, uint8_t idx) {

  palette <<= 2;

  SDL_Color col = paletteColor[ppuRead(0x3000 + palette + idx)];
  return COLOR_TO_UINT32(col);
}
