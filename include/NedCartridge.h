#ifndef __NEDCARTRIDGE_H
#define __NEDCARTRIDGE_H

#include <fstream>
#include <memory>
#include <stdint.h>
#include <vector>

namespace NedNes {
class NedMapper;
class NedBus;
class NedCartrdige {

private:
  uint8_t nPGRBanks;
  uint8_t nCHRBanks;
  std::vector<uint8_t> PGRMemory;
  std::vector<uint8_t> CHRMemory;
  std::shared_ptr<NedMapper> mMapper;
  std::shared_ptr<NedBus> BUS;

  bool valid;
  uint8_t mapperID;

public:
  bool imageValid();
  NedCartrdige(std::string filename);

  // read and writes from CPU
  uint8_t cpuRead(uint16_t addr);
  void cpuWrite(uint16_t addr, uint8_t data);

  // read and writes from PPU

  uint8_t ppuRead(uint16_t addr);
  void ppuWrite(uint16_t addr, uint8_t data);

  void connectBus(std::shared_ptr<NedBus>);
};

}; // namespace NedNes
#endif
