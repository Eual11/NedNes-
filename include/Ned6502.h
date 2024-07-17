#pragma once
#include "../include/NedBus.h"
#include <array>
#include <iostream>
#include <string>

class NedBus;
namespace NedNes {
class Ned6502 {

private:
  // bus

public:
  // registers
  uint8_t A = 0x00;    // accumulator
  uint8_t X = 0x00;    // X index register
  uint8_t Y = 0x00;    // Y index register
  uint8_t STKP = 0x00; // stack pointer
  uint16_t PC = 0x00;  // program counter
  uint8_t status = 0x00;
  NedBus *bus;

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

  bool getFlag(NedCPUFlags f);
  void setFlag(NedCPUFlags f, bool v);
  // variables to store data, address
  uint16_t absolute_addr = 0x00;
  uint16_t rel_addr = 0x00;
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

  // an instruction represents
  // mnemonic: the mnemonic for the instruction
  // addr_mode: the addressing mode for the functions

  // 6502 Operations//////////////////////////////////////////////////////////

  uint8_t BRK();
  uint8_t ORA();
  uint8_t ASL();
  uint8_t PHP();
  uint8_t BPL();
  uint8_t CLC();
  uint8_t JSR();
  uint8_t AND();
  uint8_t BIT();
  uint8_t BNE();
  uint8_t ROL();
  uint8_t PLP();
  uint8_t BMI();
  uint8_t SEC();
  uint8_t RTI();
  uint8_t EOR();
  uint8_t LSR();
  uint8_t PHA();
  uint8_t JMP();
  uint8_t BVC();
  uint8_t CLI();
  uint8_t RTS();
  uint8_t ADC();
  uint8_t ROR();
  uint8_t SEI();
  uint8_t BVS();
  uint8_t STA();
  uint8_t STY();
  uint8_t STX();
  uint8_t TYA();
  uint8_t TXS();
  uint8_t BCC();
  uint8_t LDA();
  uint8_t LDX();
  uint8_t LDY();
  uint8_t TAY();
  uint8_t TAX();
  uint8_t BCS();
  uint8_t CLV();
  uint8_t TSX();
  uint8_t CPY();
  uint8_t CMP();
  uint8_t DEC();
  uint8_t CLD();
  uint8_t CPX();
  uint8_t SBC();
  uint8_t INC();
  uint8_t INX();
  uint8_t NOP();
  uint8_t BEQ();
  uint8_t SED();
  uint8_t INY();
  uint8_t DEX();
  uint8_t DEY();
  uint8_t TXA();
  uint8_t PLA();
  uint8_t XXX();

  //////////////////////////////////////////////////////////////////////////////////
  struct INSTRUCTION {
    std::string name;
    uint8_t (Ned6502::*addr_mode)() = nullptr;
    uint8_t (Ned6502::*operate)() = nullptr;
    unsigned int cycles = 0;
  };

  using addr_mode_type = uint8_t (Ned6502::*)();
  addr_mode_type IMPLIED = &Ned6502::IMP;
  addr_mode_type ACCUMULATOR = &Ned6502::ACC;
  addr_mode_type IMMEDIATE = &Ned6502::IMM;
  addr_mode_type ZERO_PAGE = &Ned6502::ZP;
  addr_mode_type ZERO_PAGE_X = &Ned6502::ZPX;
  addr_mode_type ZERO_PAGE_Y = &Ned6502::ZPY;
  addr_mode_type RELATIVE = &Ned6502::REL;
  addr_mode_type ABSOLUTE = &Ned6502::ABS;
  addr_mode_type ABSOLUTE_X = &Ned6502::ABX;
  addr_mode_type ABSOLUTE_Y = &Ned6502::ABY;
  addr_mode_type INDIRECT = &Ned6502::IND;
  addr_mode_type INDIRECT_X = &Ned6502::IZX;
  addr_mode_type INDIRECT_Y = &Ned6502::IZY;

  INSTRUCTION opcodes[256] = {

      // 0x00
      {"BRK", IMPLIED, &Ned6502::BRK, 7},     // 0x00
      {"ORA", INDIRECT_X, &Ned6502::ORA, 6},  // 0x01
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x02
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x03
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x04
      {"ORA", ZERO_PAGE, &Ned6502::ORA, 3},   // 0x05
      {"ASL", ZERO_PAGE, &Ned6502::ASL, 5},   // 0X06
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x07
      {"PHP", IMPLIED, &Ned6502::PHP, 3},     // 0x08
      {"ORA", IMMEDIATE, &Ned6502::ORA, 2},   // 0x09
      {"ASL", ACCUMULATOR, &Ned6502::ASL, 2}, // 0x0A
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x0B
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x0C
      {"ORA", ABSOLUTE, &Ned6502::ORA, 4},    // 0X0D
      {"ASL", ABSOLUTE, &Ned6502::ASL, 6},    // 0X0E
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x0F

      // 0x10
      {"BPL", RELATIVE, &Ned6502::BPL, 2},    // 0x10
      {"ORA", INDIRECT_Y, &Ned6502::ORA, 5},  // 0x11
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x12
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x13
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x14
      {"ORA", ZERO_PAGE_X, &Ned6502::ORA, 4}, // 0X15
      {"ASL", ZERO_PAGE_X, &Ned6502::ASL, 6}, // 0X16
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x17
      {"CLC", IMPLIED, &Ned6502::CLC, 2},     // 0X18
      {"ORA", ABSOLUTE_Y, &Ned6502::ORA, 4},  // 0x19
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x1A
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x1B
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x1C
      {"ORA", ABSOLUTE_X, &Ned6502::ORA, 4},  // OX1D
      {"ASL", ABSOLUTE_X, &Ned6502::ASL, 7},  // 0x1E
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x1F

      // 0x20

  };
};

} // namespace NedNes
