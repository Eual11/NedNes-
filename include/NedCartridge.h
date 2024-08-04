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

  enum Mirror {

    HORIZONTAL,
    VERTICAL
  };

private:
  uint8_t nPGRBanks;
  uint8_t nCHRBanks;
  std::vector<uint8_t> PGRMemory;
  std::vector<uint8_t> CHRMemory;
  std::shared_ptr<NedMapper> mMapper;

  bool valid = false;
  uint8_t mapperID;

public:
  bool imageValid();
  NedCartrdige(std::string filename);
  Mirror mirror;

  // read and writes from CPU
  bool cpuRead(uint16_t addr, uint8_t &data);
  bool cpuWrite(uint16_t addr, uint8_t data);

  // read and writes from PPU

  bool ppuRead(uint16_t addr, uint8_t &data);
  bool ppuWrite(uint16_t addr, uint8_t data);
};

}; // namespace NedNes
#endif
