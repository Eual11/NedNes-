#include "../include/NedCartridge.h"
#include <fstream>
#include <stdint.h>

namespace NedNes {

NedCartrdige::NedCartrdige(std::string filename) {

  std::ifstream rom(filename);

  struct header {
    char head[3];
    uint8_t format;
    uint8_t nPGR;
    uint8_t nCHR;
    uint8_t ctrl1;
    uint8_t ctrl2;
    uint8_t nRam;
    char reserved[7];
  };

  header hd;

  rom.read((char *)&hd, sizeof(header));

  nPGRBanks = hd.nPGR;
  nCHRBanks = hd.nCHR;
  PGRMemory.resize(hd.nPGR * 16 * 1024);
  CHRMemory.resize(hd.nCHR * 8 * 1024);

  // TODO: add enumrates for mirroring
  //
  mapperID = (hd.ctrl1 >> 4) | (hd.ctrl2 & 0xF0);

  // skipping 512 Bytes trainer

  rom.seekg(512, std::ios::cur);

  rom.read((char *)PGRMemory.data(), PGRMemory.size());
  rom.read((char *)CHRMemory.data(), CHRMemory.size());

  valid = true;
  // reading
  rom.close();
}
bool NedCartrdige::imageValid() { return valid; }

}; // namespace NedNes
