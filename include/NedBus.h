#pragma once
#include "Ned2CO2.h"
#include "NedCartridge.h"
#include <array>
#include <memory>
#include <stdint.h>
#include <vector>

namespace NedNes {

class Ned6502;
class NedBus {

  // conencted devices
private:
  Ned6502 *cpu;
  std::shared_ptr<Ned2C02> ppu;
  std::shared_ptr<NedCartrdige> cart;
  //  a whooping 2KB of Nes ram!
  std::array<uint8_t, 1024 * 2> ram;

public:
  NedBus();
  void cpuWrite(uint16_t addr, uint8_t data);
  uint8_t cpuRead(uint16_t addr);
  uint8_t ppuRead(uint16_t addr);
  void ppuWrite(uint16_t addr, uint8_t data);
  void connectCpu(Ned6502 *_cpu);
  void connectPpu(std::shared_ptr<Ned2C02>);
  void connectCartridge(std::shared_ptr<NedCartrdige>);
};
} // namespace NedNes
