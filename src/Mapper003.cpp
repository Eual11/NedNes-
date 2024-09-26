#include "../include/Mapper003.h"
#include <cstdint>

NedNes::Mapper003::Mapper003(unsigned int npgr, unsigned int nchr)
    : NedMapper(npgr, nchr) {
  nPGRBanks = npgr;
  nCHRBanks = nchr;

  reset();
}

void NedNes::Mapper003::reset() { chrAddrOffset = 0x00; }

bool NedNes::Mapper003::cpuMapReadAddress(uint16_t addr, uint32_t &mapped_addr,
                                          uint8_t &data) {
  if (addr >= 0x8000 && addr <= 0xFFFF) {
    mapped_addr = (nPGRBanks == 1) ? (addr & 0x3FFF) : (addr & 0x7FFF);
    return true;
  }
  return false;
}

bool NedNes::Mapper003::cpuMapWriteAddress(uint16_t addr, uint32_t &mapped_addr,
                                           uint8_t data) {

  if (addr >= 0x8000 && addr <= 0xFFFF) {
    chrAddrOffset = data & 0xF;
  }
  return false;
}
bool NedNes::Mapper003::ppuMapReadAddress(uint16_t addr,
                                          uint32_t &mapped_addr) {
  if (addr >= 0x0000 && addr <= 0x1FFF) {
    mapped_addr = (chrAddrOffset * 0x2000) + (addr & 0x1FFF);
    return true;
  }
  return false;
}
bool NedNes::Mapper003::ppuMapWriteAddress(uint16_t addr,
                                           uint32_t &mapped_addr) {
  if (addr >= 0x0000 && addr <= 0x1FFF) {
    mapped_addr = (chrAddrOffset * 0x2000) + (addr & 0x1FFF);
    return true;
  }
  return false;
}
