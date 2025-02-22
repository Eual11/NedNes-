#include "../include/Timer.h"
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <cstdio>
#include <cstring>
#define _CRT_SECURE_NO_WARNINGS
#include "../include/Ned2CO2.h"
#include <cstdint>
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
  scrSurface = SDL_CreateRGBSurface(0, 256, 240, 32, 0xFF000000, 0X00FF0000,
                                    0X0000FF00, 0x000000FF);

  for (size_t i = 0; i < 4; i++) {
    nameTableTexture[i] =
        SDL_CreateTexture(gRenderer, SDL_PIXELFORMAT_RGBA8888,
                          SDL_TEXTUREACCESS_STREAMING, 256, 240);
  }
  if (!scrSurface) {
    printf("Couldn't Create Screen surface\n");
  } else {
    printf("Created\n");
  }
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

    break;
  }

  case 0x01: {
    // PPU Mask Registers
    break;
  }

  case 0x02: {
    // PPU Status Register
    data = (PPUSTATUS.value & 0xE0) | (buffered_data & 0x1F);

    // reseting vblank
    PPUSTATUS.bits.vblank = 0;
    // reseting PPUADDR latch
    addr_latch = 0;
    break;
  }

  case 0x03: {
    // OAM Address Register
    data = oam_addr;
    break;
  }
  case 0x04: {
    // OAM Data Register
    data = pOAM[oam_addr];
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
    buffered_data = ppuRead(v_reg.reg);

    if (v_reg.reg >= 0x3F00 && v_reg.reg < 0x3FFF) {
      // palette table doesn't rely on buffering
      data = buffered_data;
    }
    if (PPUCTRL.bits.increment_mode == 0) {
      v_reg.reg += 1;
    } else {
      v_reg.reg += 32;
    }
    break;
  }
  }
  return data;
}
void NedNes::Ned2C02::cpuWrite(uint16_t addr, uint8_t data) {
  switch (addr & 0xF) {
  case 0x00: {
    // PPU Control Register
    PPUCTRL.value = data;
    t_reg.bits.nametable_x = PPUCTRL.bits.nametable_x;
    t_reg.bits.nametable_y = PPUCTRL.bits.nametable_y;
    break;
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
    oam_addr = data;
    break;
  }
  case 0x04: {
    // OAM Data Register
    pOAM[oam_addr] = data;
    oam_addr += 1;
    break;
  }
  case 0x05: {
    // PPU Scroll register

    if (addr_latch == 0) {
      // reading fine x scroll value
      fine_x = data & 0x07;
      t_reg.bits.coarse_x = data >> 3;
      addr_latch = 1;
    } else {
      t_reg.bits.fine_y = data & 0x07;
      t_reg.bits.coarse_y = data >> 3;
      addr_latch = 0;
    }
    // toggling w register (address latch)
    break;
  }
  case 0x06: {
    // PPU Address Register
    if (addr_latch == 0) {
      // reading hi byte
      t_reg.reg = (t_reg.reg & 0x00FF) | ((uint16_t)(data & 0x3F) << 8);
      addr_latch = 1; // flipping address latch
    } else {
      t_reg.reg = (t_reg.reg & 0xFF00) | data;
      v_reg.reg = t_reg.reg;
      addr_latch = 0; // flipping address latch
    }
    break;
  }
  case 0x07: {
    // PPU Data Register

    // writing data to the address in ppu addr
    ppuWrite(v_reg.reg, data);

    // increaming ppu addr based on
    if (PPUCTRL.bits.increment_mode == 0) {
      // going across
      v_reg.reg += 1;
    } else {
      // going downward
      v_reg.reg += 32;
    }
    break;
  }
  }
}

uint8_t NedNes::Ned2C02::ppuRead(uint16_t addr) {

  uint8_t data = 0x00;

  addr &= 0x3FFF;
  if (cart->ppuRead(addr, data)) {
  } else if (addr >= 0x0000 && addr <= 0x1FFF) {
    // read from pattern table
    data = patternTable[(addr & 0x1000) >> 12][addr & 0xFFF];
  } else if (addr >= 0x2000 && addr <= 0x3EFF) {
    // nametable stuff

    addr = addr & 0x0FFF;

    switch (cart->mirror()) {
    case HORIZONTAL: {
      if (addr <= 0x7FF) {
        // mapped to the nametable 1
        data = nameTable[0][addr & 0x3FF];
      } else {
        data = nameTable[1][addr & 0x3FF];
      }

      break;
    }
    case VERTICAL: {
      if ((addr >= 0x0000 && addr <= 0x03FF) ||
          (addr >= 0x0800 && addr <= 0x0BFF)) {
        data = nameTable[0][addr & 0x03FF];
      } else if ((addr >= 0x0400 && addr <= 0x07FF) ||
                 (addr >= 0x0C00 && addr <= 0x0FFF)) {
        data = nameTable[1][addr & 0x03FF];
      }
      break;
    }
    case ONESCREEN_LO: {
      // Both 0x2000 - 0x2FFF mapped to NameTable[0]
      data = nameTable[0][addr & 0x3FF];
      break;
    }
    case ONESCREEN_HI: {
      // Both 0x2000 - 0x2FFF mapped to NameTable[1]
      data = nameTable[1][addr & 0x3FF];
      break;
    }
    }
  } else if (addr >= 0x3F00 && addr <= 0x3FFF) {
    addr &= 0x001F;
    if (addr == 0x0010)
      addr = 0x0000;
    if (addr == 0x0014)
      addr = 0x0004;
    if (addr == 0x0018)
      addr = 0x0008;
    if (addr == 0x001C)
      addr = 0x000C;
    return paletteTable[addr] & (PPUMASK.bits.greyscale ? 0x30 : 0x3F);
  }

  return data;
}
void NedNes::Ned2C02::ppuWrite(uint16_t addr, uint8_t data) {

  addr &= 0x3FFF;
  if (cart->ppuWrite(addr, data)) {
    // cartridge having veto power

  } else if (addr >= 0x0000 && addr <= 0x1FFF) {
    // write to pattern table
    patternTable[(addr & 0x1000) >> 12][addr & 0xFFF] = data;
  } else if (addr >= 0x2000 && addr <= 0x3EFF) {

    addr = addr & 0xFFF;

    switch (cart->mirrorType) {
    case HORIZONTAL: {
      if (addr <= 0x7FF) {
        // mapped to the nametable 1
        nameTable[0][addr & 0x3FF] = data;
      } else {
        nameTable[1][addr & 0x3FF] = data;
      }

      break;
    }
    case VERTICAL: {

      if ((addr >= 0x00 && addr <= 0x3FF) || (addr >= 0x800 && addr <= 0xBFF)) {
        nameTable[0][addr & 0x3FF] = data;
      } else if ((addr >= 0x400 && addr <= 0x7FF) ||
                 (addr >= 0xC00 && addr <= 0xFFF)) {
        nameTable[1][addr & 0x3FF] = data;
      }

      break;
    }
    }
  } else if (addr >= 0x3F00 && addr <= 0x3FFF) {

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
void NedNes::Ned2C02::reset() {
  fine_x = 0x00;
  v_reg.reg = 0x00;
  t_reg.reg = 0x00;
  addr_latch = 0;
  scanlines = 0;
  cycles = 0;
  next_bg_tile_id = 0x00;
  next_bg_attrib = 0x00;
  next_bg_tile_lsb = 0x00;
  next_bg_tile_msb = 0x00;
  bg_tile_shift_reg_lo = 0x00;
  bg_tile_shift_reg_hi = 0x00;
  bg_attr_shift_reg_lo = 0x00;
  bg_attr_shift_reg_hi = 0x00;
  PPUSTATUS.value = 0x00;
  PPUCTRL.value = 0x00;
}

void NedNes::Ned2C02::connectBus(std::shared_ptr<NedBus> _bus) { bus = _bus; }
void NedNes::Ned2C02::connectCart(std::shared_ptr<NedCartrdige> _cart) {
  cart = _cart;
}

void NedNes::Ned2C02::clock() {

  // advancing the clock count

  auto IncrementX = [&]() {
    if (PPUMASK.bits.bg_enable || PPUMASK.bits.sprite_enable) {
      if (v_reg.bits.coarse_x == 31) {
        v_reg.bits.coarse_x = 0;
        v_reg.bits.nametable_x = ~v_reg.bits.nametable_x;
      } else {
        v_reg.bits.coarse_x += 1;
      }
    }
  };

  auto IncrementY = [&]() {
    if (PPUMASK.bits.bg_enable || PPUMASK.bits.sprite_enable) {
      if (v_reg.bits.fine_y < 7) {
        v_reg.bits.fine_y += 1;
      } else {
        v_reg.bits.fine_y = 0;
        if (v_reg.bits.coarse_y == 29) {
          // skip to the next vertical nametable
          v_reg.bits.coarse_y = 0;
          v_reg.bits.nametable_y = ~v_reg.bits.nametable_y;
        } else if (v_reg.bits.coarse_y == 31) {
          v_reg.bits.coarse_y = 0;
        } else {
          v_reg.bits.coarse_y += 1;
        }
      }
    }
  };
  auto TransferX = [&]() {
    // NOTE: transfering X causes a messed up pallete
    if (PPUMASK.bits.bg_enable || PPUMASK.bits.sprite_enable) {
      v_reg.bits.coarse_x = t_reg.bits.coarse_x;
      v_reg.bits.nametable_x = t_reg.bits.nametable_x;
    }
  };

  auto TransferY = [&]() {
    if (PPUMASK.bits.bg_enable || PPUMASK.bits.sprite_enable) {
      v_reg.bits.coarse_y = t_reg.bits.coarse_y;
      v_reg.bits.fine_y = t_reg.bits.fine_y;
      v_reg.bits.nametable_y = t_reg.bits.nametable_y;
    }
  };

  auto UpdateShiftRegisters = [&]() {
    // Update background tile shift registers if background is enabled
    if (PPUMASK.bits.bg_enable) {
      bg_tile_shift_reg_lo <<= 1;
      bg_tile_shift_reg_hi <<= 1;
      bg_attr_shift_reg_lo <<= 1;
      bg_attr_shift_reg_hi <<= 1;
    }

    // Update sprite shift registers if sprite rendering is enabled
    if (PPUMASK.bits.sprite_enable) {
      // Only update during visible cycles for sprite evaluation
      if (cycles >= 1 && cycles <= 257) {
        for (int i = 0; i < spriteCount; i++) {
          // Check if the sprite is still within the visible area
          if (scanlineSprites[i].x > 0) {
            scanlineSprites[i].x -= 1; // Move sprite to the left
          } else {
            // Shift the sprite's shift registers
            sprite_shift_reg_lo[i] <<= 1;
            sprite_shift_reg_hi[i] <<= 1;
          }
        }
      }
    }
  };

  auto LoadShiftRegisters = [&]() {
    bg_tile_shift_reg_lo = (bg_tile_shift_reg_lo & 0xFF00) | (next_bg_tile_lsb);
    bg_tile_shift_reg_hi = (bg_tile_shift_reg_hi & 0xFF00) | (next_bg_tile_msb);

    uint8_t l_pattern = next_bg_attrib & 0b01 ? 0xFF : 0x00;
    uint8_t h_pattern = next_bg_attrib & 0b10 ? 0xFF : 0x00;

    bg_attr_shift_reg_lo = (bg_attr_shift_reg_lo & 0xFF00) | (l_pattern);
    bg_attr_shift_reg_hi = (bg_attr_shift_reg_hi & 0xFF00) | (h_pattern);
  };

  if (scanlines >= -1 && scanlines <= 239) {
    // visible scan lines

    if (scanlines == 0 && cycles == 0) {
      cycles = 1;
    }
    if (scanlines == -1 && cycles == 1) {
      PPUSTATUS.bits.vblank = 0x00;

      PPUSTATUS.bits.sprite_overflow = 0x00;
      PPUSTATUS.bits.sprite_hit = 0x00;

      for (int i = 0; i < 8; i++) {
        // resetting the shifters
        sprite_shift_reg_lo[i] = 0x00;
        sprite_shift_reg_hi[i] = 0x00;
      }
    }

    // NOTE: i think cycles should start from 1 instead of 2

    if (cycles >= 1 && cycles < 258 || cycles >= 321 && cycles <= 337) {

      UpdateShiftRegisters();
      switch ((cycles - 1) % 8) {
      case 0: {

        LoadShiftRegisters();
        // reading the next tile nametable ID
        next_bg_tile_id = ppuRead(0x2000 | (v_reg.reg & 0x0FFF));
        break;
      }
      case 2: {
        // load attribute table data for this tile

        uint16_t nx = v_reg.bits.nametable_x;
        uint16_t ny = v_reg.bits.nametable_y;
        uint16_t n_select = (ny << 1 | nx) << 10;
        uint16_t cx = v_reg.bits.coarse_x >> 2;
        uint16_t cy = (v_reg.bits.coarse_y >> 2) << 3;
        uint16_t addr = 0X23C0 | (n_select | cy | cx);
        next_bg_attrib = ppuRead(addr);
        cx = v_reg.bits.coarse_x & 0x2;
        cy = (v_reg.bits.coarse_y & 0x2);

        if (cy & 0x2)
          next_bg_attrib >>= 4;
        if (cx & 0x2)
          next_bg_attrib >>= 2;
        next_bg_attrib &= 0x03;
        break;
      }
      case 4: {
        // load LSB of pattern table tile data

        next_bg_tile_lsb =
            ppuRead((PPUCTRL.bits.bg_select << 12) +
                    (((uint16_t)next_bg_tile_id) << 4) + v_reg.bits.fine_y + 0);

        break;
      }
      case 6: {
        // Load MSB pattern table tile data

        next_bg_tile_msb =
            ppuRead((PPUCTRL.bits.bg_select << 12) |
                    (((uint16_t)next_bg_tile_id) << 4) + v_reg.bits.fine_y + 8);

        break;
      }
      case 7: {
        // increment X
        IncrementX();
        break;
      }
      }
    }

    // end of visible scanline, so we have to increament vertically
    if (cycles == 256) {
      // Increment Vertically
      IncrementY();
    }
    if (cycles == 257) {
      // Transfer Registers
      LoadShiftRegisters();
      TransferX();
    }
    if (cycles == 257 && scanlines >= 0) {
      // Sprite evaluation stage

      memset(scanlineSprites, 0xFF, 8 * sizeof(oamEntry));
      spriteCount = 0;

      for (int i = 0; i < 8; i++) {
        sprite_shift_reg_lo[i] = 0x00;
        sprite_shift_reg_hi[i] = 0x00;
      }

      spriteZeroPossible = false;
      for (int i = 0; i < 64 && spriteCount < 9; i++) {
        oamEntry sprite = oam[i];
        int8_t spriteHeight = (PPUCTRL.bits.sprite_height) ? 16 : 8;
        int16_t diff = (int16_t)scanlines - (int16_t)sprite.y;

        if (diff >= 0 && diff < spriteHeight) {
          if (spriteCount < 8) {
            if (i == 0)
              spriteZeroPossible = true;
            memcpy(&scanlineSprites[spriteCount], &oam[i], sizeof(oamEntry));
          }
          spriteCount++;
        }
      }

      // Check for sprite overflow
      if (spriteCount > 8) {
        if (PPUMASK.bits.bg_enable || PPUMASK.bits.sprite_enable)
          PPUSTATUS.bits.sprite_overflow = 1;
        spriteCount = 8;
      }
    }
    if (cycles == 340) { // Ensure the condition is true for execution
      // At the end of the current scanline, update the shifters

      for (int i = 0; i < spriteCount; i++) {
        oamEntry sprite = scanlineSprites[i];

        uint16_t sprite_addr_lo, sprite_addr_hi;
        uint16_t sprite_lo, sprite_hi;

        int16_t spriteY = scanlines - sprite.y;
        bool isSpriteHeight16 = PPUCTRL.bits.sprite_height;
        bool isVerticallyFlipped = (sprite.attrib & 0x80) != 0;
        bool isHorizontallyFlipped = (sprite.attrib & 0x40) != 0;

        if (!isSpriteHeight16) {
          // 8x8 sprite
          sprite_addr_lo = ((uint16_t)PPUCTRL.bits.sprite_select << 12) |
                           ((uint16_t)sprite.id << 4) |
                           (isVerticallyFlipped ? (7 - spriteY) : spriteY);
        } else {
          // 8x16 sprite
          if (spriteY < 8) {
            sprite_addr_lo = ((PPUCTRL.bits.sprite_select & 0x01) << 12) |
                             ((sprite.id & 0xFE) << 4) | (spriteY & 0x07);
          } else {
            sprite_addr_lo = ((PPUCTRL.bits.sprite_select & 0x01) << 12) |
                             (((sprite.id & 0xFE) + 1) << 4) | (spriteY & 0x07);
          }
          // Handle vertical flipping
          if (isVerticallyFlipped) {
            sprite_addr_lo =
                ((PPUCTRL.bits.sprite_select & 0x01) << 12) |
                (((sprite.id & 0xFE) + (spriteY < 8 ? 1 : 0)) << 4) |
                ((7 - spriteY) & 0x07);
          }
        }

        sprite_addr_hi = sprite_addr_lo + 8;

        sprite_lo = ppuRead(sprite_addr_lo);
        sprite_hi = ppuRead(sprite_addr_hi);

        // Handle horizontal flipping
        if (isHorizontallyFlipped) {
          auto reverse = [](unsigned char b) {
            b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
            b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
            b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
            return b;
          };

          sprite_lo = reverse(sprite_lo);
          sprite_hi = reverse(sprite_hi);
        }

        sprite_shift_reg_lo[i] = sprite_lo;
        sprite_shift_reg_hi[i] = sprite_hi;
      }
    }

    if (cycles == 338 || cycles == 340) {
      // superflous NT read
      next_bg_tile_id = ppuRead(0x2000 | (v_reg.reg & 0x0FFF));
    }

    if (scanlines == -1 && (cycles >= 280 || cycles <= 304)) {
      // Transfer vertical Registers
      TransferY();
    }
  }

  if (scanlines > 240 && scanlines <= 260) {

    // vblanking
    if (cycles == 1 && scanlines == 241) {
      PPUSTATUS.bits.vblank = 0x1;
      if (PPUCTRL.bits.nmi_enable) {
        nmi = true;
      }
    }
  }

  // compositon

  if ((cycles >= 1 && cycles <= 256 && scanlines >= 0 && scanlines < 240)) {

    uint16_t bg_pixel = 0x00;
    uint16_t bg_palette = 0x00;

    uint16_t fg_pixel = 0x00;
    uint16_t fg_palette = 0x00;
    uint16_t fg_priority = 0x00;

    // Final pixel and palette values
    uint8_t pixel = 0x00;
    uint8_t palette = 0x00;

    // background========================================
    if (PPUMASK.bits.bg_enable) {
      uint16_t mutx = 0x8000 >> fine_x;

      bg_pixel = ((bg_tile_shift_reg_hi & mutx) > 0) << 1 |
                 ((bg_tile_shift_reg_lo & mutx) > 0);

      bg_palette = ((bg_attr_shift_reg_hi & mutx) > 0) << 1 |
                   ((bg_attr_shift_reg_lo & mutx) > 0);
    }

    // foreground ===========================

    if (PPUMASK.bits.sprite_enable) {

      spriteZeroRendered = false;
      for (int i = 0; i < spriteCount; i++) {

        if (scanlineSprites[i].x == 0) {
          uint8_t lo, hi;

          lo = (sprite_shift_reg_lo[i] & 0x80) > 0;
          hi = (sprite_shift_reg_hi[i] & 0x80) > 0;

          fg_pixel = (hi << 1) | lo;

          fg_palette = (scanlineSprites[i].attrib & 0x03) + 0x04;

          fg_priority = (scanlineSprites[i].attrib & 0x20) == 0;

          if (fg_pixel != 0) {
            if (i == 0) {
              spriteZeroRendered = true;
            }
            break;
          }
        }
      }
    }

    // comparing transparancy of the background and the sprite
    //

    if (fg_pixel == 0 && bg_pixel == 0) {

      // both are transparant
      pixel = 0x00;
      palette = 0x00;
    } else if (fg_pixel > 0 && bg_pixel == 0) {

      // background is transparent
      pixel = fg_pixel;
      palette = fg_palette;
    }

    else if (fg_pixel == 0 && bg_pixel > 0) {
      pixel = bg_pixel;
      palette = bg_palette;
    }

    else if (fg_pixel > 0 && bg_pixel > 0) {
      // priority fight
      if (fg_priority) {
        pixel = fg_pixel;
        palette = fg_palette;
      } else {
        pixel = bg_pixel;
        palette = bg_palette;
      }

      if (spriteZeroPossible && spriteZeroRendered) {
        if (PPUMASK.bits.sprite_enable && PPUMASK.bits.bg_enable) {
          if (!(PPUMASK.bits.bg_left_enable ||
                PPUMASK.bits.sprite_left_enable)) {
            if (cycles >= 9 && cycles <= 257) {
              PPUSTATUS.bits.sprite_hit = 1;
            }
          } else {
            if (cycles >= 1 && cycles <= 257) {
              PPUSTATUS.bits.sprite_hit = 1;
            }
          }
        }
      }
    }
    if (scrSurface) {
      SET_PIXEL(scrSurface->pixels, cycles - 1, scanlines, scrSurface->pitch,

                getColorFromPalette(palette, pixel));
    }
  }

  cycles++;
  if (PPUMASK.bits.bg_enable || PPUMASK.bits.sprite_enable) {
    if (cycles == 260 && (scanlines < 240)) {
      cart->getMapper()->scanline();
    }
  }

  frameComplete = false;
  if (cycles >= 341) {
    cycles = 0;
    scanlines++;
    if (scanlines >= 261) {
      scanlines = -1;
      frameComplete = true;
    }
  }
}

SDL_Texture *NedNes::Ned2C02::getScreenTexture() {

  Uint32 *pixels;
  int pitch;
  SDL_LockTexture(screenTexture, nullptr, (void **)&pixels, &pitch);

  memcpy((void *)pixels, scrSurface->pixels, scrSurface->pitch * scrSurface->h);
  SDL_UnlockTexture(screenTexture);
  return screenTexture;
}
SDL_Texture *NedNes::Ned2C02::getPatternTable(uint8_t i, uint8_t palette) {

  Uint32 *pixels;
  int pitch;
  SDL_Texture *tex = patternTableTexture[i];

  SDL_LockTexture(tex, nullptr, (void **)&pixels, &pitch);
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

SDL_Texture *NedNes::Ned2C02::getNameTable(uint8_t i, uint8_t palette) {

  uint8_t idx = i & 0x03;
  SDL_Texture *tex = nameTableTexture[idx];

  Uint32 *pixels = nullptr;
  int pitch;

  SDL_LockTexture(tex, nullptr, (void **)(&pixels), &pitch);

  uint16_t offset = 0x2000 | ((uint16_t)(idx) << 10);

  for (int YTile = 0; YTile < 30; YTile++) {
    for (int XTile = 0; XTile < 32; XTile++) {

      uint16_t addr = (YTile << 5) + XTile;

      uint16_t tileID = ppuRead(addr + offset);

      for (int row = 0; row < 8; row++) {
        uint8_t tile_lsb =
            ppuRead((PPUCTRL.bits.bg_select << 12) + (tileID << 4) + 0 + row);
        uint8_t tile_msb =
            ppuRead((PPUCTRL.bits.bg_select << 12) + (tileID << 4) + 8 + row);

        for (int col = 0; col < 8; col++) {

          uint8_t pixel = (tile_lsb & 0x01) | ((tile_msb & 0x01) << 1);

          int yPos = YTile * 8 + row;
          int xPos = XTile * 8 + (7 - col);

          if (pixels) {
            SET_PIXEL(pixels, xPos, yPos, pitch,
                      getColorFromPalette(palette, pixel));
          }
          tile_lsb >>= 1;
          tile_msb >>= 1;
        }
      }
    }
  }

  SDL_UnlockTexture(tex);
  return tex;
}
Uint32 NedNes::Ned2C02::getColorFromPalette(uint8_t palette, uint8_t idx) {

  palette <<= 2;

  SDL_Color col = paletteColor[ppuRead(0x3F00 + palette + idx)];
  return COLOR_TO_UINT32(col);
}
uint8_t NedNes::Ned2C02::getOamAddr() { return oam_addr; }
