#pragma once
#include <array>
#include <stdint.h>
#include <vector>

namespace NedNes {

class Ned6502;
class NedBus {

  // conencted devices
private:
  Ned6502 *cpu;
  // 64KB fake ram
  std::array<uint8_t, 1024 * 64> ram;

public:
  NedBus();
  void write(uint16_t addr, uint8_t data);
  uint8_t read(uint16_t addr);
  void ConnectCpu(Ned6502 *_cpu);
};
} // namespace NedNes
