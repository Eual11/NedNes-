#include "../include/Mapper004.h"

NedNes::Mapper004::Mapper004(unsigned int npgr, unsigned int nchr)
    : NedMapper(npgr, nchr) {

  nPGRBanks = npgr;
  nCHRBanks = nchr;

  RAM.reserve(32 * 1024);

  // reseting the mapper to a known statesA
  reset();
}

bool NedNes::Mapper004::cpuMapReadAddress(uint16_t addr, uint32_t &mapped_addr,
                                          uint8_t &data) {

  // we split the ROM to 8KB banks, the offset for each bank is stored in
  // nPGRBanks array and depending on the read region we fetch the appropriate
  // value

  if (addr >= 0x6000 && addr <= 0x7FFF) {

    // reading from the RAM if it's enabled
    if (!ramDisabled) {
      mapped_addr = 0xFFFFFFFF;

      data = RAM[addr & 0x1FFF];
      return true;
    }
  } else if (addr >= 0x8000 && addr <= 0x9FFF) {

    mapped_addr = PGRBanks[0] + (addr & 0x1FFF);
    return true;
  } else if (addr >= 0xA000 && addr <= 0xBFFF) {
    mapped_addr = PGRBanks[1] + (addr & 0x1FFF);
    return true;
  } else if (addr >= 0xC000 && addr <= 0xDFFF) {
    mapped_addr = PGRBanks[2] + (addr & 0x1FFF);
    return true;
  } else if (addr >= 0xE000 && addr <= 0xFFFF) {
    mapped_addr = PGRBanks[3] + (addr & 0x1FFF);
    return true;
  }
  return false;
}
bool NedNes::Mapper004::ppuMapReadAddress(uint16_t addr,
                                          uint32_t &mapped_addr) {
  // again we split the Characeter ROM to 8 1k banks  handle them each

  if (addr >= 0x0000 && addr <= 0x3FF) {
    mapped_addr = CHRBanks[0] + (addr & 0x3FF);
    return true;
  } else if (addr >= 0x0400 && addr <= 0x07FF) {
    mapped_addr = CHRBanks[1] + (addr & 0x3FF);
    return true;
  } else if (addr >= 0x0800 && addr <= 0x0BFF) {
    mapped_addr = CHRBanks[2] + (addr & 0x3FF);
    return true;
  } else if (addr >= 0x0C00 && addr <= 0x0FFF) {
    mapped_addr = CHRBanks[3] + (addr & 0x3FF);
    return true;
  } else if (addr >= 0x1000 && addr <= 0x13FF) {
    mapped_addr = CHRBanks[4] + (addr & 0x3FF);
    return true;
  } else if (addr >= 0x1400 && addr <= 0x17FF) {
    mapped_addr = CHRBanks[5] + (addr & 0x3FF);
    return true;
  } else if (addr >= 0x1800 && addr <= 0x1BFF) {
    mapped_addr = CHRBanks[6] + (addr & 0x3FF);
    return true;
  } else if (addr >= 0x1C00 && addr <= 0x1FFF) {
    mapped_addr = CHRBanks[7] + (addr & 0x3FF);
    return true;
  }
  return false;
}
bool NedNes::Mapper004::ppuMapWriteAddress(uint16_t addr,
                                           uint32_t &mapped_addr) {
  return false;
}
bool NedNes::Mapper004::cpuMapWriteAddress(uint16_t addr, uint32_t &mapped_addr,
                                           uint8_t data) {
  if (addr >= 0x6000 && addr <= 0x7FFF) {
    if (!ramWriteDisabled) {
      RAM[addr & 0x1FFF] = data;
      mapped_addr = 0xFFFFFFFF;
      return true;
    }
  }
  if (addr >= 0x8000 && addr <= 0x9FFF) {
    // bank select
    if (!(addr & 0x0001)) {

      TargetReg = data & 0x07;
      PGRMode = (data & 0x40);
      CHRInversion = (data & 0x80);
    } else {

      Registers[TargetReg] = data;

      // Updating the pointer table
      //

      if (CHRInversion) {
        CHRBanks[0] = (Registers[2] * 0x400);
        CHRBanks[1] = (Registers[3] * 0x400);
        CHRBanks[2] = (Registers[4] * 0x400);
        CHRBanks[3] = (Registers[5] * 0x400);
        CHRBanks[4] = (Registers[0] & 0xFE) * 0x400;
        CHRBanks[5] = (Registers[0] * 0x400) + 0x400;
        CHRBanks[6] = (Registers[1] & 0xFE) * 0x400;
        CHRBanks[7] = (Registers[1] * 0x400) + 0x400;

      } else {

        CHRBanks[0] = (Registers[0] & 0xFE) * 0x400;
        CHRBanks[1] = (Registers[0] * 0x400) + 0x400;
        CHRBanks[2] = (Registers[1] & 0xFE) * 0x400;
        CHRBanks[3] = (Registers[1] * 0x400) + 0x400;
        CHRBanks[4] = (Registers[2] * 0x400);
        CHRBanks[5] = (Registers[3] * 0x400);
        CHRBanks[6] = (Registers[4] * 0x400);
        CHRBanks[7] = (Registers[5] * 0x400);
      }
      if (PGRMode) {
        PGRBanks[0] = (nPGRBanks * 2 - 2) * (0x2000);
        PGRBanks[2] = (Registers[6] & 0x3F) * 0x2000;
      } else {
        PGRBanks[0] = (Registers[6] & 0x3F) * 0x2000;
        PGRBanks[2] = (nPGRBanks * 2 - 2) * (0x2000);
      }
      PGRBanks[1] = (Registers[7] & 0x3F) * 0x2000;
      PGRBanks[3] = (nPGRBanks * 2 - 1) * (0x2000);
    }
  } else if (addr >= 0xA000 && addr <= 0xBFFF) {
    // mirroring and RAM protect
    if (!(addr & 0x0001)) {
      mirrorType = (data & 0x01) ? HORIZONTAL : VERTICAL;
    } else {
      ramDisabled = (data & 0x80) == 0;
      ramWriteDisabled = (data & 0x40) == 1;
    }
  } else if (addr >= 0xC000 && addr <= 0xDFFF) {

    if (!(addr & 0x0001)) {
      // IRQ latch
      IRQReload = data;
    } else {
      // IRQ Reload
      IRQCounter = 0x00;
    }

  } else if (addr >= 0xE000 && addr <= 0xFFFF) {
    // Enable /Disable IRQ
    if (!(addr & 0x0001)) {
      IRQEnable = false;
      IRQActive = false;

    } else {
      IRQEnable = true;
    }
  }

  return false;
}

void NedNes::Mapper004::scanline() {
  if (IRQCounter == 0) {
    IRQCounter = IRQReload;
  } else
    IRQCounter--;

  if (IRQCounter == 0 && IRQEnable) {
    IRQActive = true;
  }
}
void NedNes::Mapper004::reset() {
  TargetReg = 0x00;
  PGRMode = false;
  CHRInversion = false;
  mirrorType = HORIZONTAL;

  IRQActive = false;
  IRQEnable = false;
  IRQCounter = 0x00;
  IRQReload = 0x00;
  for (int i = 0; i < 4; i++)
    PGRBanks[i] = 0;
  for (int i = 0; i < 8; i++) {
    CHRBanks[i] = 0;
    Registers[i] = 0;
  }

  PGRBanks[0] = 0 * 0x2000;
  PGRBanks[1] = 1 * 0x2000;
  PGRBanks[2] = (nPGRBanks * 2 - 2) * 0x2000;
  PGRBanks[3] = (nPGRBanks * 2 - 1) * 0x2000;
}
