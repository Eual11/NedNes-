#ifndef __NED2CO2__H
#define __NED2CO2__H

// Ned's PPU: 2C02
#include <cstdint>
#include <memory>
#include <stdint.h>
namespace NedNes {
class NedBus;
class NedCartrdige;
class Ned2C02 {

public:
  // cpu read and writes
  // these methods are called from the bus handle read and write signals from
  // cpu

  Ned2C02();
  uint8_t cpuRead(uint16_t addr);
  void cpuWrite(uint16_t addr, uint8_t data);

  // PPU read and writes

  uint8_t ppuRead(uint8_t addr);
  void ppuWrite(uint16_t addr, uint8_t data);

  // connecting to cartage and BUS

  // scanlines and cycles

  std::shared_ptr<NedBus> bus;
  std::shared_ptr<NedCartrdige> cart;

  void connectBus(std::shared_ptr<NedBus>);
  void connectCart(std::shared_ptr<NedCartrdige>);

private:
  unsigned int cycles = 0;
  unsigned int scanlines = 0;

  uint8_t paletteTable[32];   // 32Byte  palette
  uint8_t nameTable[4][1024]; // name table used to store background information
  uint8_t patternTable[2][1024]; // pattern table, this is located inside th chr
                                 // rom but i am storing it here for now
};
}; // namespace NedNes

#endif
