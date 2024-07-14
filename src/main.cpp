#include <functional>
#include <iostream>
#include <stdint.h>

struct INSTRUCTION {
  std::string mnemonic;
  std::function<uint8_t()> addr_mode = nullptr;
  std::function<uint8_t()> operate = nullptr;
  unsigned int cycles = 0;
};

uint8_t nop() {
  // Implementation of the 'nop' function
  return 0;
}

int main() {
  INSTRUCTION NOP;
  NOP.mnemonic = "NOP";
  NOP.operate = nop;

  return 0;
}
