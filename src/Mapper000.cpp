#include "../include/Mapper000.h"
#include <cstdint>

namespace NedNes {

Mapper000::Mapper000(unsigned int npgr, unsigned int nchr)
    : NedMapper(npgr, nchr) {
  nPGRBanks = npgr;
  nCHRBanks = nchr;
}

bool Mapper000::cpuMapReadAddress(uint16_t addr, uint32_t &mapped,
                                  uint8_t &data) {

  if (addr >= 0x8000 && addr <= 0xFFFF) {
    mapped = nPGRBanks == 1 ? (addr & 0x3FFF) : (addr & 0x7FFF);
    return true;
  }
  return false;
}

bool Mapper000::cpuMapWriteAddress(uint16_t addr, uint32_t &mapped,
                                   uint8_t data) {

  if (addr >= 0x8000 && addr <= 0xFFFF) {
    mapped = nPGRBanks == 1 ? (addr & 0x3FFF) : (addr & 0x7FFF);
    return true;
  }
  return false;
}

bool Mapper000::ppuMapWriteAddress(uint16_t addr, uint32_t &mapped) {

  if (addr >= 0x0000 && addr <= 0x1FFF) {
    mapped = (addr & 0x1FFF);
    return true;
  }
  return false;
}
bool Mapper000::ppuMapReadAddress(uint16_t addr, uint32_t &mapped) {

  if (addr >= 0x0000 && addr <= 0x1FFF) {
    mapped = (addr & 0x1FFF);
    return true;
  }
  return false;
}
void Mapper000::reset() {
  // do nothing
}

}; // namespace NedNes
