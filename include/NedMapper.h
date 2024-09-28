#ifndef __NEDMAPPER_H
#define __NEDMAPPER_H

#include <stdint.h>
enum Mirror {

  HARDWARE,
  HORIZONTAL,
  VERTICAL,
  ONESCREEN_LO,
  ONESCREEN_HI,
};
namespace NedNes {

class NedMapper {

public:
  NedMapper(unsigned int npgr, unsigned int nchr);
  virtual ~NedMapper() = default;

  virtual bool cpuMapReadAddress(uint16_t addr, uint32_t &mapped,
                                 uint8_t &data) = 0;
  virtual bool cpuMapWriteAddress(uint16_t addr, uint32_t &mapped,
                                  uint8_t data) = 0;
  virtual bool ppuMapReadAddress(uint16_t addr, uint32_t &mapped) = 0;
  virtual bool ppuMapWriteAddress(uint16_t addr, uint32_t &mapped) = 0;
  virtual void reset() = 0; // resets the mapper to a known state

  void setMirror(Mirror mirror) { mirrorType = mirror; }
  Mirror getMirror() { return mirrorType; }

protected:
  unsigned int nPGRBanks;
  unsigned int nCHRBanks;
  Mirror mirrorType = HORIZONTAL;
};
}; // namespace NedNes
#endif
