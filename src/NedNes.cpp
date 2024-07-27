#include "../include/NedNes.h"
#include <ios>
#include <stdint.h>

NedNes::NedNesEmulator::NedNesEmulator() {
  nedBus.ConnectCpu(&nedCpu);
  nedCpu.connectBus(&nedBus);
}
bool NedNes::NedNesEmulator::loadRom(std::string path, uint16_t start_addr) {

  std::ifstream rom(path, std::ios::binary);
  if (!rom.is_open()) {
    fprintf(stderr, "Couldn't Open Room");
    return false;
  }

  std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(rom)),
                              std::istreambuf_iterator<char>());

  printf("%zu\n", buffer.size());
  int idx = 0x10;

  for (size_t addr = start_addr; addr <= 0xffff; addr++) {
    if (idx < buffer.size()) {
      nedBus.write(addr, buffer[idx++]);
    }
  }

  return true;
}
