#ifndef __MAPPER003__
#define __MAPPER003__

#include "NedMapper.h"
#include <cstdint>
namespace NedNes {
class Mapper003 : public NedMapper {

public:
  Mapper003(unsigned int npgr, unsigned int nchr);
  ~Mapper003() override = default;

  bool cpuMapReadAddress(uint16_t addr, uint32_t &mapped_addr,
                         uint8_t &data) override;
  bool cpuMapWriteAddress(uint16_t addr, uint32_t &mapped_addr,
                          uint8_t data) override;
  bool ppuMapReadAddress(uint16_t addr, uint32_t &mapped_addr) override;
  bool ppuMapWriteAddress(uint16_t addr, uint32_t &mapped_addr) override;
  void reset() override;
  void irqClear() override {};
  void scanline() override {};
  bool irqState() override { return false; }

private:
  uint8_t chrAddrOffset = 0x00;
};
}; // namespace NedNes
#endif
