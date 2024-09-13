#ifndef __NED2CO2__H
#define __NED2CO2__H

// Ned's PPU: 2C02
#include "../include/NedCartridge.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <cstdint>
#include <memory>
#include <stdint.h>

#define SET_PIXEL(pixel_ptr, x, y, pitch, color)                               \
  ((Uint32 *)(pixel_ptr))[(y * ((pitch) / 4)) + (x)] = (color)
#define COLOR_TO_UINT32(color)                                                 \
  (((color).r << 24) | ((color).g << 16) | ((color).b << 8) | (color).a)
namespace NedNes {
class NedBus;
class Ned2C02 {

public:
  // cpu read and writes
  // these methods are called from the bus handle read and write signals from
  // cpu

  // represents the layout of Control Register
  union {

    struct {
      uint8_t nametable_x : 1;
      uint8_t nametable_y : 1;
      uint8_t increment_mode : 1;
      uint8_t sprite_select : 1;
      uint8_t bg_select : 1;
      uint8_t sprite_height : 1;
      uint8_t master_slave : 1;
      uint8_t nmi_enable : 1;
    } bits;
    uint8_t value;
  } PPUCTRL;
  // Layout of PPU Mask Register or CTRL2

  union {

    struct {
      uint8_t greyscale : 1;
      uint8_t bg_left_enable : 1;
      uint8_t sprite_left_enable : 1;
      uint8_t bg_enable : 1;
      uint8_t sprite_enable : 1;
      uint8_t color_emphasis : 3;
    } bits;
    uint8_t value;
  } PPUMASK;

  uint8_t OAMADDR;   // oam r/w addresssA
  uint8_t OAMDATA;   // OAM R/W DATA
  uint8_t PPUSCROLL; // scroll register
  /* uint16_t PPUADDR;  // ppu address */
  uint8_t PPUDATA; // data register
  uint8_t OAMDMA;
  // Layout of PPU status register
  uint8_t buffered_data = 0x00;

  bool addr_latch =
      0; // the tracks if the PPUAddr is reading lo or hi byte on the next write
  union {
    struct {
      uint8_t unused : 5;
      uint8_t sprite_overflow : 1;
      uint8_t sprite_hit : 1;
      uint8_t vblank : 1;
    } bits;
    uint8_t value;
  } PPUSTATUS;

  uint8_t next_bg_tile_id = 0x00;
  uint8_t next_bg_attrib = 0x00;
  uint8_t next_bg_tile_lsb = 0x00;
  uint8_t next_bg_tile_msb = 0x00;

  // shift registers
  //
  uint16_t bg_tile_shift_reg_lo = 0x0000;
  uint16_t bg_tile_shift_reg_hi = 0x0000;
  uint16_t bg_attr_shift_reg_lo = 0x0000;
  uint16_t bg_attr_shift_reg_hi = 0x0000;

  Ned2C02(SDL_Renderer *gRenderer);
  ~Ned2C02();
  uint8_t cpuRead(uint16_t addr);
  bool nmi = false;
  void cpuWrite(uint16_t addr, uint8_t data);

  // PPU read and writes

  uint8_t ppuRead(uint16_t addr);
  void ppuWrite(uint16_t addr, uint8_t data);

  union loopy_reg {
    struct {

      uint8_t coarse_x : 5;
      uint8_t coarse_y : 5;
      uint8_t nametable_x : 1;
      uint8_t nametable_y : 1;
      uint8_t fine_y : 3;
      uint8_t unused : 1;
    } bits;

    uint16_t reg;
  };
  loopy_reg v_reg;
  loopy_reg t_reg;
  uint8_t fine_x;
  // ppu clock
  void clock();

  // connecting to cartage and BUS

  // scanlines and cycles

  std::shared_ptr<NedBus> bus;
  std::shared_ptr<NedCartrdige> cart;

  void connectBus(std::shared_ptr<NedBus>);
  void connectCart(std::shared_ptr<NedCartrdige>);
  SDL_Texture *getScreenTexture();
  SDL_Texture *getPatternTable(uint8_t i, uint8_t palette);

  SDL_Texture *getNameTable(uint8_t i, uint8_t palette);
  Uint32 getColorFromPalette(uint8_t palette, uint8_t idx);
  bool isFrameComplete() { return frameComplete; };

private:
  int cycles = 0;
  int scanlines = 0;

  unsigned int clockCount = 0;
  bool frameComplete = false;

  uint8_t paletteTable[32];   // 32Byte  palette
  uint8_t nameTable[2][1024]; // name table used to store background information
  uint8_t patternTable[2][4 * 1024]; // pattern table, this is located inside th
                                     // chr rom but i am storing it here for now
  // the renderered Screen texture
  SDL_Texture *screenTexture = nullptr;
  SDL_Renderer *renderer = nullptr;
  SDL_Texture *patternTableTexture[2];
  SDL_Texture *nameTableTexture[4];
  SDL_Surface *scrSurface = nullptr;
  SDL_Color paletteColor[0x40] = {
      {0x7C, 0x7C, 0x7C, 0xFF}, {0x00, 0x00, 0xFC, 0xFF},
      {0x00, 0x00, 0xBC, 0xFF}, {0x44, 0x28, 0xBC, 0xFF},
      {0x94, 0x00, 0x84, 0xFF}, {0xA8, 0x00, 0x20, 0xFF},
      {0xA8, 0x10, 0x00, 0xFF}, {0x88, 0x14, 0x00, 0xFF},
      {0x50, 0x30, 0x00, 0xFF}, {0x00, 0x78, 0x00, 0xFF},
      {0x00, 0x68, 0x00, 0xFF}, {0x00, 0x58, 0x00, 0xFF},
      {0x00, 0x40, 0x58, 0xFF}, {0x00, 0x00, 0x00, 0xFF},
      {0x00, 0x00, 0x00, 0xFF}, {0x00, 0x00, 0x00, 0xFF},
      {0xBC, 0xBC, 0xBC, 0xFF}, {0x00, 0x78, 0xF8, 0xFF},
      {0x00, 0x58, 0xF8, 0xFF}, {0x68, 0x44, 0xFC, 0xFF},
      {0xD8, 0x00, 0xCC, 0xFF}, {0xE4, 0x00, 0x58, 0xFF},
      {0xF8, 0x38, 0x00, 0xFF}, {0xE4, 0x5C, 0x10, 0xFF},
      {0xAC, 0x7C, 0x00, 0xFF}, {0x00, 0xB8, 0x00, 0xFF},
      {0x00, 0xA8, 0x00, 0xFF}, {0x00, 0xA8, 0x44, 0xFF},
      {0x00, 0x88, 0x88, 0xFF}, {0x00, 0x00, 0x00, 0xFF},
      {0x00, 0x00, 0x00, 0xFF}, {0x00, 0x00, 0x00, 0xFF},
      {0xF8, 0xF8, 0xF8, 0xFF}, {0x3C, 0xBC, 0xFC, 0xFF},
      {0x68, 0x88, 0xFC, 0xFF}, {0x98, 0x78, 0xF8, 0xFF},
      {0xF8, 0x78, 0xF8, 0xFF}, {0xF8, 0x58, 0x98, 0xFF},
      {0xF8, 0x78, 0x58, 0xFF}, {0xFC, 0xA0, 0x44, 0xFF},
      {0xF8, 0xB8, 0x00, 0xFF}, {0xB8, 0xF8, 0x18, 0xFF},
      {0x58, 0xD8, 0x54, 0xFF}, {0x58, 0xF8, 0x98, 0xFF},
      {0x00, 0xE8, 0xD8, 0xFF}, {0x78, 0x78, 0x78, 0xFF},
      {0x00, 0x00, 0x00, 0xFF}, {0x00, 0x00, 0x00, 0xFF},
      {0xFC, 0xFC, 0xFC, 0xFF}, {0xA4, 0xE4, 0xFC, 0xFF},
      {0xB8, 0xB8, 0xF8, 0xFF}, {0xD8, 0xB8, 0xF8, 0xFF},
      {0xF8, 0xB8, 0xF8, 0xFF}, {0xF8, 0xA4, 0xC0, 0xFF},
      {0xF0, 0xD0, 0xB0, 0xFF}, {0xFC, 0xE0, 0xA8, 0xFF},
      {0xF8, 0xD8, 0x78, 0xFF}, {0xD8, 0xF8, 0x78, 0xFF},
      {0xB8, 0xF8, 0xB8, 0xFF}, {0xB8, 0xF8, 0xD8, 0xFF},
      {0x00, 0xFC, 0xFC, 0xFF}, {0xF8, 0xD8, 0xF8, 0xFF},
      {0x00, 0x00, 0x00, 0xFF}, {0x00, 0x00, 0x00, 0xFF}};
};
}; // namespace NedNes

#endif
