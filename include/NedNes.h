#ifndef __NEDNES__H
#define __NEDNES__H

#include "Ned2A03.h"
#include "Ned2CO2.h"
#include "Ned6502.h"
#include "NedBus.h"
#include "NedCartridge.h"
#include <SDL2/SDL_audio.h>
#include <cstdint>
#include <fstream>
#include <memory>
#include <string>

namespace NedNes {
class NedNesEmulator {

private:
  std::shared_ptr<Ned6502> CPU = nullptr;
  std::shared_ptr<Ned2C02> PPU;
  std::shared_ptr<Ned2A03> APU;
  std::shared_ptr<NedBus> EmuBus;
  std::shared_ptr<NedCartrdige> cart;
  std::shared_ptr<NedJoypad> joypad1;
  std::shared_ptr<NedJoypad> joypad2;

public:
  NedNesEmulator() = default;

  NedNesEmulator(SDL_Renderer *gRenderer, SDL_AudioDeviceID id);
  NedNesEmulator(SDL_Renderer *gRenderer, std::string, SDL_AudioDeviceID id);
  bool loadRom(std::string);
  void unload();
  void reset();
  void stepCycle();
  void stepFrame();
  SDL_Texture *getNewFrame();
  void setControllerState(uint8_t n, uint8_t state);
  std::map<uint16_t, std::string> getDissmap();
  std::shared_ptr<NedBus> getBus() const { return EmuBus; };
  std::shared_ptr<Ned6502> getCPU() const { return CPU; }
  std::shared_ptr<Ned2C02> getPPU() const { return PPU; }
  std::shared_ptr<Ned2A03> getAPU() const { return APU; }

  void fillAudioBuffer(int16_t *audio_buffer, unsigned int size) {
    APU->fillAudioBuffer(audio_buffer, size);
  }
};
} // namespace NedNes
#endif
