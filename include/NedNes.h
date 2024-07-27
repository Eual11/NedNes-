#ifndef __NEDNES__H
#define __NEDNES__H

#include "Ned6502.h"
#include "NedBus.h"
#include <fstream>
#include <string>

namespace NedNes {
class NedNesEmulator {
public:
  NedNes::NedBus nedBus;
  NedNes::Ned6502 nedCpu;

public:
  NedNesEmulator();
  bool loadRom(std::string, uint16_t start_addr);
};
} // namespace NedNes
#endif
