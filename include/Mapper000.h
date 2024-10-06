#ifndef __MAPPER000__
#define __MAPPER000__

#include "NedMapper.h"

#include <cstdint>
namespace NedNes {
class Mapper000 : public NedMapper {
public:
  Mapper000(unsigned int npgr, unsigned int nchr);
  ~Mapper000() override = default;

  bool cpuMapReadAddress(uint16_t addr, uint32_t &mapped,
                         uint8_t &data) override;
  bool cpuMapWriteAddress(uint16_t addr, uint32_t &mapped,
                          uint8_t data) override;
  bool ppuMapReadAddress(uint16_t addr, uint32_t &mapped) override;
  bool ppuMapWriteAddress(uint16_t addr, uint32_t &mapped) override;
  void reset() override;
  void irqClear() override {};
  void scanline() override {};
  bool irqState() override { return false; }
};
}; // namespace NedNes

#endif
