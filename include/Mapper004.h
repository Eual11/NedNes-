#ifndef __MAPPER004__
#define __MAPPER004__

#include "NedMapper.h"
#include <cstdint>
#include <vector>

namespace NedNes {

class Mapper004 : public NedMapper {
public:
  Mapper004(unsigned int, unsigned int);
  ~Mapper004() override = default;

  bool cpuMapReadAddress(uint16_t addr, uint32_t &mapped_addr,
                         uint8_t &data) override;
  bool cpuMapWriteAddress(uint16_t addr, uint32_t &mapped_addr,
                          uint8_t data) override;

  bool ppuMapWriteAddress(uint16_t addr, uint32_t &mapped_addr) override;
  bool ppuMapReadAddress(uint16_t addr, uint32_t &mapped_addr) override;
  void reset() override;
  void irqClear() override { IRQActive = false; };
  void scanline() override;
  bool irqState() override { return IRQActive; }

private:
  std::vector<uint8_t> RAM;

  uint8_t TargetReg = 0x00;
  bool CHRInversion = false;
  bool PGRMode = false;
  bool IRQEnable = false;
  bool IRQActive = false;

  uint16_t IRQCounter = 0x00;
  uint16_t IRQReload = 0x00;
  bool ramDisabled = false;

  bool ramWriteDisabled = false;

  uint32_t Registers[8];
  uint32_t PGRBanks[4];
  uint32_t CHRBanks[8];
};
}; // namespace NedNes

#endif
