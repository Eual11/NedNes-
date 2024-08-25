#include "../include/NedCartridge.h"
#include "../include/Mapper000.h"
#include <fstream>
#include <iostream>
#include <memory>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

namespace NedNes {

NedCartrdige::NedCartrdige(std::string filename) {

  std::ifstream rom(filename, std::ios::binary);
  if (rom.is_open()) {

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

    printf("Format: %X\n", hd.format);
    printf("NPGR: %d\n", (int)hd.nPGR);
    printf("NCHR: %d\n", (int)hd.nCHR);
    nPGRBanks = hd.nPGR;
    nCHRBanks = hd.nCHR;
    PGRMemory.resize(hd.nPGR * 16 * 1024);
    CHRMemory.resize(hd.nCHR * 8 * 1024);

    // TODO: add enumrates for mirroring
    //
    if (hd.format == 0x1A) {
      // INES FORMAT
      mapperID = (hd.ctrl1 >> 4) | (hd.ctrl2 & 0xF0);
      printf("PGR Banks: %d, CHR Banks:  %d, Mapper ID: %d \n", nPGRBanks,
             nCHRBanks, mapperID);

      switch (mapperID) {
      case 0x00: {
        mMapper = std::make_shared<Mapper000>(nPGRBanks, nCHRBanks);
        break;
      }

      default: {
        fprintf(stderr,
                "Mapper %d hasn't been Implemented Yet, Please Be Patient ^^\n",
                mapperID);
        exit(1);
        break;
      }
      }

      // skipping 512 Bytes trainer

      if (hd.ctrl1 & 0x04)
        rom.seekg(512, std::ios::cur);
      rom.read((char *)PGRMemory.data(), PGRMemory.size());
      rom.read((char *)CHRMemory.data(), CHRMemory.size());

      valid = true;
    }
    rom.close();
  }
  if (!valid) {
    fprintf(stderr, "Failed to Load Rom, Invalid ROM or Unsupported Format\n");
  }
}
bool NedCartrdige::imageValid() { return valid; }
bool NedCartrdige::cpuRead(uint16_t addr, uint8_t &data) {
  // mapping the cpu address to the phsical program memory address in the room
  // using the mapper
  uint32_t mapped_addr;
  if (mMapper->cpuMapReadAddress(addr, mapped_addr)) {
    // reading from the program memory
    data = PGRMemory[mapped_addr];
  }
  return false;
}

bool NedCartrdige::cpuWrite(uint16_t addr, uint8_t data) {

  // mapping cpu address to the physical memoery address in the rom
  uint32_t mapped_addr;

  if (mMapper->cpuMapWriteAddress(addr, mapped_addr)) {
    PGRMemory[mapped_addr] = data;
    return true;
  }
  return false;
}

bool NedCartrdige::ppuRead(uint16_t addr, uint8_t &data) {
  uint32_t mapped_addr;
  if (mMapper->ppuMapReadAddress(addr, mapped_addr)) {

    data = CHRMemory[mapped_addr];
    return true;
  }
  return false;
}
bool NedCartrdige::ppuWrite(uint16_t addr, uint8_t data) {

  uint32_t mapped_addr;

  if (mMapper->ppuMapWriteAddress(addr, mapped_addr)) {
    CHRMemory[mapped_addr] = data;
    return true;
  }
  return false;
}
}; // namespace NedNes
