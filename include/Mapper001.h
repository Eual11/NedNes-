#ifndef __MAPPER001__
#define __MAPPER001__
#include "NedMapper.h"
#include <cstdint>
#include <vector>

namespace NedNes {
class Mapper001 : public NedMapper {
public:
  Mapper001(unsigned int npgr, unsigned int nchr);
  ~Mapper001() override = default;

  bool cpuMapReadAddress(uint16_t addr, uint32_t &mapped,
                         uint8_t &data) override;

  bool cpuMapWriteAddress(uint16_t addr, uint32_t &mapped,
                          uint8_t data) override;

  bool ppuMapReadAddress(uint16_t addr, uint32_t &mapped) override;

  bool ppuMapWriteAddress(uint16_t addr, uint32_t &mapped) override;

  void reset() override;

private:
  std::vector<uint8_t> RAM;

  // bank selector for 4k banj switching
  uint8_t CHRSelectBank4Lo;
  uint8_t CHRSelectBank4Hi;

  // for 8k mode
  uint8_t CHRSelectBank8k;

  uint8_t PGRSelectBank16kLo;
  uint8_t PGRSelectBank16kHi;

  // 32k mode
  uint8_t PGRSelectBank32K;
  uint8_t ControlReg; // control register
  uint8_t LoaderReg;  // loader register
  uint8_t LoaderCount = 0x00;
};
}; // namespace NedNes
#endif
