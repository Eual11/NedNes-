#include "../include/Mapper001.h"
#include <cstdint>
void NedNes::Mapper001::reset() {
  CHRSelectBank8k = 0x00;
  CHRSelectBank4Hi = 0x00;
  CHRSelectBank4Lo = 0x00;

  PGRSelectBank32K = 0x00;
  PGRSelectBank16kLo = 0x00;
  PGRSelectBank16kHi = nPGRBanks - 1;

  LoaderReg = 0x00;
  LoaderCount = 0x00;
  ControlReg = 0x1C;
}
NedNes::Mapper001::Mapper001(unsigned int npgr, unsigned int nchr)
    : NedMapper(npgr, nchr) {

  nPGRBanks = npgr;
  nCHRBanks = nchr;

  // some respectable amount of ram
  RAM.reserve(32 * 1024);
  reset();
}

bool NedNes::Mapper001::cpuMapReadAddress(uint16_t addr, uint32_t &mapped_addr,
                                          uint8_t &data) {
  if (addr >= 0x6000 && addr <= 0x7FFF) {
    mapped_addr = 0xFFFFFFFF;

    data = RAM[addr & 0x1FFF];
    return true;
  }

  if (addr >= 0x8000 && addr <= 0xFFFF) {

    if (ControlReg & 0b01000) {
      // checking for the mode of cpu

      // bank switching time
      if (addr >= 0x8000 && addr <= 0xBFFF) {
        mapped_addr = ((uint16_t)PGRSelectBank16kLo * 0x4000) + (addr & 0x3FFF);
      } else if (addr >= 0xC000 && addr <= 0xFFFF) {
        mapped_addr = ((uint16_t)PGRSelectBank16kHi * 0x4000) + (addr & 0x3FFF);
      }
    } else {

      mapped_addr = 0x8000 * (uint16_t)PGRSelectBank32K + (addr & 0x7FFF);
    }

    return true;
  }
  return false;
}

bool NedNes::Mapper001::cpuMapWriteAddress(uint16_t addr, uint32_t &mapped,
                                           uint8_t data) {

  if (addr >= 0x6000 && addr <= 0x7FFF) {

    RAM[addr & 0x1FFF] = data;
    mapped = 0xFFFFFFFF;
    return true;
  }

  if (addr >= 0x8000 && addr <= 0xFFFF) {
    // if data's msb is set then reset the load register and control reg as well
    if (data & 0x80) {

      LoaderReg = 0x00;
      LoaderCount = 0x00;
      ControlReg = ControlReg | 0xC;
    } else {

      LoaderReg >>= 1;
      LoaderReg |= (data & 0x1) << 4;
      LoaderCount++;
      /* printf("%X LoaderReg\n", LoaderReg); */

      if (LoaderCount == 5) {

        /* printf("Final Loader Reg: %X\n", LoaderReg); */
        uint16_t targetReg = (addr >> 13) & (0x3);

        switch (targetReg) {
        case 0x00: {
          ControlReg = LoaderReg & 0x1F;

          uint8_t table = ControlReg & 0x3;

          if (table == 0)
            mirrorType = ONESCREEN_LO;
          if (table == 1)
            mirrorType = ONESCREEN_HI;
          if (table == 2)
            mirrorType = VERTICAL;
          if (table == 3)
            mirrorType = HORIZONTAL;
          break;
        }

        case 0x01: {

          // switch the low bank of chr memory depending on the mode
          if (ControlReg & 0b10000) {
            CHRSelectBank4Lo = (LoaderReg & 0x1F);
          } else
            CHRSelectBank8k = (LoaderReg & 0x1E);
          break;
        }
        case 0x02: {
          if (ControlReg & 0b10000) {
            CHRSelectBank4Hi = (LoaderReg & 0x1F);
          }
          break;
        }
        case 0x3: {

          uint8_t pgrMode = (ControlReg >> 2) & (0x3);

          if (pgrMode == 0 || pgrMode == 1) {
            PGRSelectBank32K = (LoaderCount & 0xE) >> 1;
          } else if (pgrMode == 2) {
            PGRSelectBank16kLo = 0x00;
            PGRSelectBank16kHi = LoaderReg & 0xF;
          } else if (pgrMode == 3) {
            PGRSelectBank16kHi = nPGRBanks - 1;
            PGRSelectBank16kLo = LoaderReg & 0xf;
          }
          break;
        }
        }

        LoaderReg = 0x00;
        LoaderCount = 0;
      }
    }
  }
  return false;
}
bool NedNes::Mapper001::ppuMapWriteAddress(uint16_t addr,
                                           uint32_t &mapped_addr) {
  if (addr >= 0x0000 && addr <= 0x1FFF) {
    if (nCHRBanks == 0) { // CHR-RAM scenario
      mapped_addr = addr; // Map directly to CHR-RAM
      return true;
    }
    // Do nothing for CHR-ROM writes, which should be read-only.
  }
  return false;
}

bool NedNes::Mapper001::ppuMapReadAddress(uint16_t addr,
                                          uint32_t &mapped_addr) {

  if (addr >= 0x0000 && addr <= 0x1FFF) {
    if (nCHRBanks == 0) {
      mapped_addr = addr;
    }
    return true;
  } else {

    if (ControlReg & 0b10000) {
      // 4K CHR Bank Mode
      if (addr >= 0x0000 && addr <= 0x0FFF) {
        mapped_addr = (uint16_t)CHRSelectBank4Lo * 0x1000 + (addr & 0x0FFF);
        return true;
      }

      if (addr >= 0x1000 && addr <= 0x1FFF) {
        mapped_addr = (uint16_t)CHRSelectBank4Hi * 0x1000 + (addr & 0x0FFF);
        return true;
      }
    } else {
      // 8K CHR Bank Mode
      mapped_addr = (uint16_t)CHRSelectBank8k * 0x2000 + (addr & 0x1FFF);
      return true;
    }
  }
  return false;
}
