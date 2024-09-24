// INES mapper 2
//
#ifndef __MAPPER002__
#define __MAPPER002__

#include "NedMapper.h"
#include <cstdint>
namespace NedNes {
class Mapper002 : public NedMapper {
public:
  Mapper002(unsigned int npgr, unsigned int nchr);
  ~Mapper002() override = default;

  bool cpuMapReadAddress(uint16_t addr, uint32_t &mapped,
                         uint8_t &data) override;

  bool cpuMapWriteAddress(uint16_t addr, uint32_t &mapped,
                          uint8_t data) override;

  bool ppuMapReadAddress(uint16_t addr, uint32_t &mapped) override;

  bool ppuMapWriteAddress(uint16_t addr, uint32_t &mapped) override;

  void reset() override;

private:
  uint8_t addrLwoffset = 0x00;
  uint8_t addrHiOffset = 0x00;
};
}; // namespace NedNes
#endif
