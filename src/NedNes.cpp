#include "../include/NedNes.h"
#include <SDL2/SDL_audio.h>
#include <cstdint>
#include <memory>
#include <stdint.h>

NedNes::NedNesEmulator::NedNesEmulator(SDL_Renderer *gRenderer,
                                       SDL_AudioDeviceID id) {
  joypad1 = std::make_shared<NedNes::NedJoypad>();
  joypad2 = std::make_shared<NedNes::NedJoypad>();
  // setting up nednes bus
  EmuBus = std::make_shared<NedNes::NedBus>();
  CPU = std::make_shared<NedNes::Ned6502>();
  PPU = std::make_shared<NedNes::Ned2C02>(gRenderer);
  APU = std::make_shared<NedNes::Ned2A03>(id);
  cart = std::make_shared<NedCartrdige>();
  PPU->connectBus(EmuBus);
  PPU->connectCart(cart);
  EmuBus->connectCartridge(cart);
  EmuBus->connectPpu(PPU);
  EmuBus->connectCpu(CPU);
  EmuBus->connectApu(APU);
  EmuBus->connectJoypad(0, joypad1);
  EmuBus->connectJoypad(1, joypad2);
  CPU->connectBus(EmuBus);
  reset();
}
NedNes::NedNesEmulator::NedNesEmulator(SDL_Renderer *gRenderer,
                                       std::string path, SDL_AudioDeviceID id) {
  joypad1 = std::make_shared<NedNes::NedJoypad>();
  joypad2 = std::make_shared<NedNes::NedJoypad>();
  // setting up nednes bus
  EmuBus = std::make_shared<NedNes::NedBus>();
  CPU = std::make_shared<NedNes::Ned6502>();
  PPU = std::make_shared<NedNes::Ned2C02>(gRenderer);
  APU = std::make_shared<NedNes::Ned2A03>(id);
  cart = std::make_shared<NedCartrdige>(path);
  PPU->connectBus(EmuBus);
  PPU->connectCart(cart);
  EmuBus->connectCartridge(cart);
  EmuBus->connectPpu(PPU);
  EmuBus->connectCpu(CPU);
  EmuBus->connectApu(APU);
  EmuBus->connectJoypad(0, joypad1);
  EmuBus->connectJoypad(1, joypad2);
  CPU->connectBus(EmuBus);
  reset();
}
bool NedNes::NedNesEmulator::loadRom(std::string path) {
  cart->unload();
  bool loaded = cart->loadRom(path);
  EmuBus->reset();
  return loaded;
}
void NedNes::NedNesEmulator::setControllerState(uint8_t n, uint8_t state) {
  EmuBus->setState(n, state);
}
void NedNes::NedNesEmulator::stepCycle() {
  if (!cart->imageValid())
    return;
  do {
    EmuBus->clock();

  } while (!EmuBus->cpu->complete());
  do {
    EmuBus->clock();

  } while (EmuBus->cpu->complete());
}
void NedNes::NedNesEmulator::stepFrame() {
  if (!cart->imageValid())
    return;
  while (!EmuBus->ppu->isFrameComplete()) {
    EmuBus->clock();
  }
  while (!EmuBus->cpu->complete()) {
    EmuBus->clock();
  }
  while (EmuBus->cpu->complete()) {
    EmuBus->clock();
  }
}
SDL_Texture *NedNes::NedNesEmulator::getNewFrame() {
  return PPU->getScreenTexture();
}
void NedNes::NedNesEmulator::unload() { cart->unload(); }
std::map<uint16_t, std::string> NedNes::NedNesEmulator::getDissmap() {

  return CPU->disassemble(4);
}
void NedNes::NedNesEmulator::reset() { EmuBus->reset(); }
