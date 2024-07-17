#include "../include/Ned6502.h"
using namespace NedNes;
bool Ned6502::getFlag(Ned6502::NedCPUFlags f) { return (status & f) > 0; }
void NedNes::Ned6502::setFlag(Ned6502::NedCPUFlags f, bool v) {
  if (v)
    status |= f;
  else
    status &= ~f;
}

Ned6502::Ned6502(NedBus *_bus) { bus = _bus; }
uint8_t Ned6502::read(uint16_t addr) { return bus->read(addr); }
void Ned6502::write(uint16_t addr, uint8_t val) { bus->write(addr, val); }
uint8_t Ned6502::fetch(uint16_t addr) {
  // TODO: add a check for the current addressing mode
  fetched = read(addr);
  return fetched;
}

uint8_t Ned6502::IMP() {
  fetched = A;
  return 0;
}
uint8_t Ned6502::ACC() {
  fetched = A;
  return 0x00;
}
uint8_t Ned6502::IMM() {

  absolute_addr = PC++;
  return 0x00;
}
uint8_t Ned6502::ZP() {
  absolute_addr = read(PC++);
  absolute_addr &= 0x00FF;
  return 0x00;
}
uint8_t Ned6502::ZPX() {
  absolute_addr = read(PC++) + X;
  absolute_addr &= 0x00FF;

  return 0x00;
}
uint8_t Ned6502::ZPY() {
  absolute_addr = read(PC++) + Y;
  absolute_addr &= 0x00FF;
  return 0x00;
}
uint8_t Ned6502::ABS() {
  uint8_t lo = read(PC++);
  uint8_t hi = read(PC++);

  absolute_addr = ((hi << 8) | lo);
  return 0x00;
}
uint8_t Ned6502::ABX() {
  uint8_t lo = read(PC++);
  uint8_t hi = read(PC++);

  absolute_addr = ((hi << 8) | lo) + X;
  absolute_addr &= 0xFFFF;
  return ((absolute_addr >> 8) & 0xFF) == ((hi)&0xFF);
}
uint8_t Ned6502::ABY() {
  uint8_t lo = read(PC++);
  uint8_t hi = read(PC++);

  absolute_addr = ((hi << 8) | lo) + Y;
  absolute_addr &= 0xFFFF;
  return ((absolute_addr >> 8) & 0xFF) == ((hi)&0xFF);
}

uint8_t Ned6502::IND() {
  uint8_t lo = read(PC++);
  uint8_t hi = read(PC++);

  uint16_t ptr = (hi << 8) | lo;

  // simulating hardware bug
  if (lo == 0xFF) {
    // simualte hardware bug
    absolute_addr = (read(ptr & 0xFF00) << 8 | read(ptr + 0));
  }

  else {
    // behave normally
    absolute_addr = ((read(ptr + 1) << 8) | read(ptr + 0));
  }

  return 0;
}

uint8_t Ned6502::IZX() {

  uint16_t offset = read(PC++);

  uint16_t lOfset = (offset + X) & 0x00FF;
  uint16_t hOfset = (offset + X + 1) & 0x00FF;

  absolute_addr = (read(hOfset << 8) | read(lOfset));

  return 0x00;
}
uint8_t Ned6502::IZY() {
  uint16_t t = read(PC++);

  uint16_t hi = read((t + 1)) << 8;
  uint16_t lo = read(t);

  absolute_addr = hi | lo;
  absolute_addr += Y;

  return (hi & 0xFF) == (absolute_addr & 0xFF);
}

uint8_t Ned6502::REL() {
  rel_addr = read(PC++);
  if (rel_addr & 0x80) {
    rel_addr |= 0xFF00;
  }
  return 0x0;
}

// INSTRUCTIONS

uint8_t Ned6502::XXX() {
  std::cout << "Not Implemented\n";
  return 0x00;
}
