#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_stdinc.h>
#include <cstdint>
#define _CRT_SECURE_NO_WARNINGS
#include "../include/Ned2CO2.h"
#include <memory>

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
  return 0xFF;
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

uint8_t NedNes::Ned2C02::ppuRead(uint16_t addr) {

  uint8_t data = 0x00;

  if (cart->ppuRead(addr, data)) {
  } else if (addr >= 0x0000 && addr <= 0x1FFF) {
    // read from pattern table

    data = patternTable[(addr & 0x1000) >> 12][addr & 0xFFF];
  } else if (addr >= 0x2000 && addr <= 0x2FFF) {
    // nametable stuff
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

  // do something idk yet lmao
}

void NedNes::Ned2C02::connectBus(std::shared_ptr<NedBus> _bus) { bus = _bus; }
void NedNes::Ned2C02::connectCart(std::shared_ptr<NedCartrdige> _cart) {
  cart = _cart;
}

void NedNes::Ned2C02::clock() {

  // advancing the clock count
  clockCount = 0;

  cycles++;

  if (cycles >= 341) {
    cycles = 0;
    scanlines++;
  }
  if (scanlines >= 261) {
    scanlines = -1;
    frameComplete = true;
  }
}
SDL_Texture *NedNes::Ned2C02::getScreenTexture() { return screenTexture; }
SDL_Texture *NedNes::Ned2C02::getPatternTable(uint8_t i, uint8_t palette) {

  Uint32 *pixels;
  int pitch;
  SDL_Texture *tex = patternTableTexture[i];

  SDL_LockTexture(tex, nullptr, (void **)(&pixels), &pitch);
  int pixels_wr = 0;
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
          SET_PIXEL(pixels, xPos, yPos, pitch, getColorFromPalette(palette, i));
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
