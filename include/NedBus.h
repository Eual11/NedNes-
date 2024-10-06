#pragma once
#include "../include/Ned6502.h"
#include "Ned2CO2.h"
#include "NedCartridge.h"
#include "NedJoypad.h"
#include <array>
#include <cstdint>
#include <memory>
#include <stdint.h>
#include <vector>

namespace NedNes {

class Ned6502;
class NedBus {

  // conencted devices
private:
  //  a whooping 2KB of Nes ram!
  std::array<uint8_t, 1024 * 2> ram;

public:
  NedBus();
  void cpuWrite(uint16_t addr, uint8_t data);
  uint8_t cpuRead(uint16_t addr);
  uint8_t ppuRead(uint16_t addr);
  void ppuWrite(uint16_t addr, uint8_t data);
  void connectCpu(std::shared_ptr<Ned6502>);
  void connectPpu(std::shared_ptr<Ned2C02>);
  void connectCartridge(std::shared_ptr<NedCartrdige>);
  void setState(int n, uint8_t state);
  void connectJoypad(int n, std::shared_ptr<NedJoypad>);
  std::shared_ptr<Ned6502> cpu = nullptr;
  std::shared_ptr<Ned2C02> ppu = nullptr;
  std::shared_ptr<NedCartrdige> cart = nullptr;
  std::shared_ptr<NedJoypad> joypads[2] = {nullptr, nullptr};

  // NOTE: seems reasonable for now
  void Press(int n, JOYPAD_BUTTONS btn);
  void Release(int n, JOYPAD_BUTTONS btn);
  void clock();
  void reset();
  uint32_t SystemClock = 0;

  // used for dma
  bool dma_transfer = false;
  bool dma_pre = true;
  uint8_t dma_page = 0x00;
  uint16_t dma_addr = 0x00;
  uint8_t dma_data = 0x00;
  uint16_t dma_transfered_data = 0;
};
} // namespace NedNes
