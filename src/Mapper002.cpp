#include "../include/Mapper002.h"
#include <cstdint>

namespace NedNes {
Mapper002::Mapper002(unsigned int npgr, unsigned int nchr)
    : NedMapper(npgr, nchr) {
  nPGRBanks = npgr;
  nCHRBanks = nchr;

  reset();
}

bool Mapper002::cpuMapReadAddress(uint16_t addr, uint32_t &mapped_addr,
                                  uint8_t &data) {

  // switchable PGR banks
  //
  if (addr >= 0x8000 && addr <= 0xBFFF) {
    mapped_addr = (uint32_t)addrLwoffset * 0x4000 + (addr & 0x3FFF);
  }
  if (addr >= 0xC000 && addr <= 0xFFFF) {
    // Fixed bank
    mapped_addr = (uint32_t)addrHiOffset * 0x4000 + (addr * 0x3FFF);
    return true;
  }

  return false;
}
bool Mapper002::cpuMapWriteAddress(uint16_t addr, uint32_t &mapped_addr,
                                   uint8_t data) {
  if (addr >= 0x8000 && addr <= 0xFFFF) {
    addrLwoffset = data & 0xF;
    return true;
  }
  return false;
}

bool Mapper002::ppuMapReadAddress(uint16_t addr, uint32_t &mapped_addr) {

  if (addr >= 0x0000 && addr <= 0x1FFF) {
    mapped_addr = (addr & 0x1FFF);
    return true;
  }
  return false;
}
bool Mapper002::ppuMapWriteAddress(uint16_t addr, uint32_t &mapped_addr) {

  if (addr >= 0x0000 && addr <= 0x1FFF) {
    mapped_addr = (addr & 0x1FFF);
    return true;
  }
  return false;
}

void Mapper002::reset() {
  addrLwoffset = 0;
  addrHiOffset = nPGRBanks - 1;
  if (nPGRBanks == 0)
    addrHiOffset = 0;
}
}; // namespace NedNes
