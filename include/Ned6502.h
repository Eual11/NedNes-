#pragma once
#include <array>
#include <cstdint>
#include <functional>
#include <string>

namespace NedNes {
class NedBus;
class Ned6502 {

private:
  // bus
  NedBus *bus;

public:
  // registers
  uint8_t A = 0x00;    // accumulator
  uint8_t X = 0x00;    // X index register
  uint8_t Y = 0x00;    // Y index register
  uint8_t STKP = 0x00; // stack pointer
  uint16_t PC = 0x00;  // program counter
  uint8_t status = 0x00;

  Ned6502(NedBus *_bus);
  enum NedCPUFlags {
    C = (1 << 0), // CARRY FLAG
    Z = (1 << 1), // Zero Flag
    I = (1 << 2), // Interupt Disable Flag
    D = (1 << 3), // Decimal Mode (Unused because the Nes does not use it)
    B = (1 << 4), // BreakCommand Flag
    U = (1 << 5), // Unused
    V = (1 << 6), // Overflow Flag
    N = (1 << 7), // Negative Flag
  };

  bool getFlag(NedCPUFlags f) { return (status & f) > 0; }
  void setFlag(NedCPUFlags f, bool v) {
    if (v) {
      status |= f;
    } else
      status &= ~f;
  }

  // variables to store data, address
  uint16_t addr = 0x00;
  uint8_t rel_addr = 0x00;
  uint8_t fetched = 0x00;
  uint8_t opcode = 0x00; // currently executing opcode

  // read and write functions

  uint8_t read(uint16_t addr);
  void write(uint16_t addr, uint8_t val);

  unsigned int cycles = 0; // cpu cycles
  //

  // interrupts
  void reset();
  void nmi();
  void irq();

  // the ticking clock;
  void clock();

  // fills the fetched variable by the correct data
  uint8_t fetch(uint16_t addr);

  // Addressing Modes, The 6502 had +12 Addressing modes to data and operands

  uint8_t IMP(); // Implied Addressing mode there is no operand
  uint8_t ACC(); // Accumulator Addressing Mode: the accumulator is the operand
  uint8_t IMM(); // Immediate Addressing Mode: the operand is supplied in the
                 // instruction
  uint8_t ZP(); // Zero Page Addressing Mode: the operand is a memory address in
                // the Zero page memory ($0000-00FF)
  uint8_t ZPX(); // Zero Page X Indexed, the effective address is calculated by
                 // adding the zero page address to the X index register (It
                 // wraps around zero page during overflow)
  uint8_t
  ZPY(); // Zero Page Y indexed, same us ZPX but uses the Y index register
  uint8_t
  ABS(); // Absolute Addressing Mode, the operand is the full 16 bit address
  uint8_t ABX(); // X Indexed Absolute Addressing Mode: the effective address is
                 // calculated by adding the Absolute address and the value in
                 // the X register
  uint8_t ABY(); // Y indexed Absolute Addressing Mode: Similar to ABX but used
                 // the Y register instead
  uint8_t IND(); // Indirect Addressing Mode, the operand is a pointer to where
                 // the data is located
  uint8_t
  IZX(); // Indirect X index Addressing the the operand is found in the memory
         // address we get by adding the zero page address and X register
  uint8_t IZY(); // Indirect Y Indexed Addressing Mode, Similar to IZX but used
                 // the y register
  uint8_t REL(); // Relative Addressing Mode: The second operand is an offset
                 // value from -128 to 127 that will be added to the PC, this is
                 // used by branching instruction
};

// an instruction represents
// mnemonic: the mnemonic for the instruction
// addr_mode: the addressing mode for the functions
struct INSTRUCTION {
  std::string mnemonic;
  std::function<uint8_t()> addr_mode = nullptr;
  std::function<uint8_t()> operate = nullptr;
  unsigned int cycles = 0;
};

} // namespace NedNes
