#ifndef __NEDCARTRIDGE_H
#define __NEDCARTRIDGE_H

#include "Mapper000.h"
#include "NedMapper.h"
#include <fstream>
#include <memory>
#include <stdint.h>
#include <vector>

namespace NedNes {
class NedCartrdige {

private:
  uint8_t nPGRBanks;
  uint8_t nCHRBanks;
  std::vector<uint8_t> PGRMemory;
  std::vector<uint8_t> CHRMemory;
  std::shared_ptr<NedMapper> mMapper = nullptr;

  bool valid = false;
  uint8_t mapperID;

public:
  bool imageValid();
  NedCartrdige(std::string filename);
  Mirror mirrorType;

  // read and writes from CPU
  bool cpuRead(uint16_t addr, uint8_t &data);
  bool cpuWrite(uint16_t addr, uint8_t data);

  // read and writes from PPU

  bool ppuRead(uint16_t addr, uint8_t &data);
  bool ppuWrite(uint16_t addr, uint8_t data);

  std::shared_ptr<NedMapper> getMapper() { return mMapper; }
  Mirror mirror() {
    if (mMapper) {
      return mMapper->getMirror();
    }
    // this shouln't happen but assuming it's horizintally mirrored seems better
    return HORIZONTAL;
  }
};

}; // namespace NedNes
#endif
