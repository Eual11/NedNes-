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
      {"JSR", ABSOLUTE, &Ned6502::JSR, 6},    // 0X20
      {"AND", INDIRECT_X, &Ned6502::AND, 6},  // 0X21
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x22
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x23
      {"BIT", ZERO_PAGE, &Ned6502::BIT, 3},   // 0x24
      {"AND", ZERO_PAGE, &Ned6502::AND, 3},   // 0x25
      {"ROL", ZERO_PAGE, &Ned6502::ROL, 5},   // 0X26
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x27
      {"PLP", IMPLIED, &Ned6502::PLP, 4},     // 0x28
      {"AND", IMMEDIATE, &Ned6502::AND, 2},   // 0x29
      {"ROL", ACCUMULATOR, &Ned6502::ROL, 2}, // 0x2A
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x2B
      {"BIT", ABSOLUTE, &Ned6502::BIT, 4},    // 0x2C
      {"AND", ABSOLUTE, &Ned6502::AND, 4},    // 0x2D
      {"ROL", ABSOLUTE, &Ned6502::ROL, 6},    // 0x2E
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x2F

      // 0x30
      {"BMI", RELATIVE, &Ned6502::BMI, 2},    // 0x30
      {"AND", INDIRECT_Y, &Ned6502::AND, 5},  // 0x31
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x32
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x33
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x34
      {"AND", ZERO_PAGE_X, &Ned6502::AND, 4}, // 0x35
      {"ROL", ZERO_PAGE_X, &Ned6502::ROL, 6}, // 0x36
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x37
      {"SEC", IMPLIED, &Ned6502::SEC, 2},     // 0x38
      {"AND", ABSOLUTE_Y, &Ned6502::AND, 4},  // 0x39
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x3A
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x3B
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x3C
      {"AND", ABSOLUTE_X, &Ned6502::AND, 4},  // 0x3D
      {"ROL", ABSOLUTE_X, &Ned6502::ROL, 7},  // 0x3E
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x3F

      // 0x40
      {"RTI", IMPLIED, &Ned6502::RTI, 6},     // 0x40
      {"EOR", INDIRECT_X, &Ned6502::EOR, 6},  // 0x41
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x42
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x43
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x44
      {"EOR", ZERO_PAGE, &Ned6502::EOR, 3},   // 0x45
      {"LSR", ZERO_PAGE, &Ned6502::LSR, 5},   // 0x46
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x47
      {"PHA", IMPLIED, &Ned6502::PHA, 3},     // 0x48
      {"EOR", IMMEDIATE, &Ned6502::EOR, 2},   // 0x49
      {"LSR", ACCUMULATOR, &Ned6502::LSR, 2}, // 0x4A
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x4B
      {"JMP", ABSOLUTE, &Ned6502::JMP, 3},    // 0x4C
      {"EOR", ABSOLUTE, &Ned6502::EOR, 4},    // 0x4D
      {"LSR", ABSOLUTE, &Ned6502::LSR, 6},    // 0x4E
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x4F

      // 0X50
      {"BVC", RELATIVE, &Ned6502::BVC, 2},    //-0x50
      {"EOR", INDIRECT_Y, &Ned6502::EOR, 5},  // 0x51
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x52
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x53
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x54
      {"EOR", ZERO_PAGE_X, &Ned6502::EOR, 4}, // 0x55
      {"LSR", ZERO_PAGE_X, &Ned6502::LSR, 6}, // 0x56
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x57
      {"CLI", IMPLIED, &Ned6502::CLI, 2},     // 0x58
      {"EOR", ABSOLUTE_Y, &Ned6502::EOR, 4},  // 0x59
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x5A
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x5B
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x5C
      {"EOR", ABSOLUTE_X, &Ned6502::EOR, 4},  // 0x5D
      {"LSR", ABSOLUTE_X, &Ned6502::LSR, 7},  // 0x5E
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x5F

      // 0x60
      {"RTS", IMPLIED, &Ned6502::RTS, 6},     // 0x60
      {"ADC", INDIRECT_X, &Ned6502::ADC, 6},  // 0x61
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x62
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x63
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x64
      {"ADC", ZERO_PAGE, &Ned6502::ADC, 3},   // 0x65
      {"ROR", ZERO_PAGE, &Ned6502::ROR, 5},   // 0x66
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x67
      {"PLA", IMPLIED, &Ned6502::PLA, 4},     // 0x68
      {"ADC", IMMEDIATE, &Ned6502::ADC, 2},   // 0x69
      {"ROR", ACCUMULATOR, &Ned6502::ROR, 2}, // 0x6A
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x6B
      {"JMP", INDIRECT, &Ned6502::JMP, 5},    // 0x6C
      {"ADC", ABSOLUTE, &Ned6502::ADC, 4},    //-0x6D
      {"ROR", ABSOLUTE, &Ned6502::ROR, 6},    // 0x6E
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x6F

      //-0x70
      {"BVS", RELATIVE, &Ned6502::BVS, 2},    // 0x70
      {"ADC", INDIRECT_Y, &Ned6502::ADC, 5},  // 0x71
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x72
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x73
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x74
      {"ADC", ZERO_PAGE_X, &Ned6502::ADC, 4}, // 0x75
      {"ROR", ZERO_PAGE_X, &Ned6502::ROR, 6}, // 0x76
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x77
      {"SEI", IMPLIED, &Ned6502::SEI, 2},     // 0x78
      {"ADC", ABSOLUTE_Y, &Ned6502::ADC, 4},  // 0x79
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x7A
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x7B
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x7C
      {"ADC", ABSOLUTE_X, &Ned6502::ADC, 4},  // 0x7D
      {"ROR", ABSOLUTE_X, &Ned6502::ROR, 7},  // 0x7E
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x7F

      // 0x80

      {"XXX", IMPLIED, &Ned6502::XXX},       // 0x80
      {"STA", INDIRECT_X, &Ned6502::STA, 6}, // 0x81
      {"XXX", IMPLIED, &Ned6502::XXX},       // 0x82
      {"XXX", IMPLIED, &Ned6502::XXX},       // 0x83
      {"STY", ZERO_PAGE, &Ned6502::STY, 3},  // 0x84
      {"STA", ZERO_PAGE, &Ned6502::STA, 3},  // 0x85
      {"STX", ZERO_PAGE, &Ned6502::STX, 3},  // 0x86
      {"XXX", IMPLIED, &Ned6502::XXX},       // 0x87
      {"DEY", IMPLIED, &Ned6502::DEY, 2},    // 0x88
      {"XXX", IMPLIED, &Ned6502::XXX},       // 0x89
      {"TXA", IMPLIED, &Ned6502::TXA, 2},    // 0x8A
      {"XXX", IMPLIED, &Ned6502::XXX},       // 0x8B
      {"STY", ABSOLUTE, &Ned6502::STY, 4},   // 0x8C
      {"STA", ABSOLUTE, &Ned6502::STA, 4},   // 0x8D
      {"STX", ABSOLUTE, &Ned6502::STX, 4},   // 0x8E
      {"XXX", IMPLIED, &Ned6502::XXX},       // 0x8F

      //-0x90
      {"BCC", RELATIVE, &Ned6502::BCC, 2},    // 0x90
      {"STA", INDIRECT_Y, &Ned6502::STA, 6},  // 0x91
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x92
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x93
      {"STY", ZERO_PAGE_X, &Ned6502::STY, 4}, // 0x94
      {"STA", ZERO_PAGE_X, &Ned6502::STA, 4}, // 0x95
      {"STX", ZERO_PAGE_Y, &Ned6502::STX, 4}, // 0x96
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x97
      {"TYA", IMPLIED, &Ned6502::TYA, 2},     // 0x98
      {"STA", ABSOLUTE_Y, &Ned6502::STA, 5},  // 0x99
      {"TXS", IMPLIED, &Ned6502::TXS, 2},     // 0x9A
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x9B
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x9C
      {"STA", ABSOLUTE_X, &Ned6502::STA, 5},  //-0x9D
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x9E
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0x9F

      // 0XA0
      {"LDY", IMMEDIATE, &Ned6502::LDY, 2},  // 0xA0
      {"LDA", INDIRECT_X, &Ned6502::LDA, 6}, // 0XA1
      {"LDX", IMMEDIATE, &Ned6502::LDX, 2},  // 0xA2
      {"XXX", IMPLIED, &Ned6502::XXX},       // 0xA3
      {"LDY", ZERO_PAGE, &Ned6502::LDY, 3},  // 0xA4
      {"LDA", ZERO_PAGE, &Ned6502::LDA, 3},  // 0xA5
      {"LDX", ZERO_PAGE, &Ned6502::LDX, 3},  // 0xA6
      {"XXX", IMPLIED, &Ned6502::XXX},       // 0xA7
      {"TAY", IMPLIED, &Ned6502::TAY, 2},    // 0xA8
      {"LDA", IMMEDIATE, &Ned6502::LDA, 2},  // 0XA9
      {"TAX", IMPLIED, &Ned6502::TAX, 2},    // 0xAA
      {"XXX", IMPLIED, &Ned6502::XXX},       // 0xAB
      {"LDY", ABSOLUTE, &Ned6502::LDY, 4},   // 0xAC
      {"LDA", ABSOLUTE, &Ned6502::LDA, 4},   // 0xAD
      {"LDX", ABSOLUTE, &Ned6502::LDX, 4},   // 0xAE
      {"XXX", IMPLIED, &Ned6502::XXX},       // 0xAF

      // 0xB0
      {"BCS", RELATIVE, &Ned6502::BCS, 2},    // 0xB0
      {"LDA", INDIRECT_Y, &Ned6502::LDA, 5},  // 0xB1
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0xB2
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0xB3
      {"LDY", ZERO_PAGE_X, &Ned6502::LDY, 4}, // 0xB4
      {"LDA", ZERO_PAGE_X, &Ned6502::LDA, 4}, // 0xB5
      {"LDX", ZERO_PAGE_Y, &Ned6502::LDX, 4}, // 0xB6
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0xB7
      {"CLV", IMPLIED, &Ned6502::CLV, 2},     // 0xB8
      {"LDA", ABSOLUTE_Y, &Ned6502::LDA, 4},  //-0xB9
      {"TSX", IMPLIED, &Ned6502::TSX, 2},     // 0xBA
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0xBB
      {"LDY", ABSOLUTE_X, &Ned6502::LDY, 4},  // 0xBC
      {"LDA", ABSOLUTE_X, &Ned6502::LDA, 4},  // 0xBD
      {"LDX", ABSOLUTE_Y, &Ned6502::LDX, 4},  // 0xBE
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0xBF

      // 0xC0

      {"CPY", IMMEDIATE, &Ned6502::CPY, 2},  // 0xC0
      {"CMP", INDIRECT_X, &Ned6502::CMP, 6}, // 0xC1
      {"XXX", IMPLIED, &Ned6502::XXX},       // 0xC2
      {"XXX", IMPLIED, &Ned6502::XXX},       // 0xC3
      {"CPY", ZERO_PAGE, &Ned6502::CPY, 3},  // 0xC4
      {"CMP", ZERO_PAGE, &Ned6502::CMP, 3},  // 0xC5
      {"DEC", ZERO_PAGE, &Ned6502::DEC, 5},  // 0xC6
      {"XXX", IMPLIED, &Ned6502::XXX},       // 0xC7
      {"INY", IMPLIED, &Ned6502::INY, 2},    // 0xC8
      {"CMP", IMMEDIATE, &Ned6502::CMP, 2},  // 0xC9
      {"DEX", IMPLIED, &Ned6502::DEX, 2},    // 0xCA
      {"XXX", IMPLIED, &Ned6502::XXX},       // 0xCB
      {"CPY", ABSOLUTE, &Ned6502::CPY, 4},   // 0xCC
      {"CMP", ABSOLUTE, &Ned6502::CMP, 4},   // 0xCD
      {"DEC", ABSOLUTE, &Ned6502::DEC, 6},   // 0xCE
      {"XXX", IMPLIED, &Ned6502::XXX},       // 0xCF

      // 0xD0

      {"BNE", RELATIVE, &Ned6502::BNE, 2},    // 0xD0
      {"CMP", INDIRECT_Y, &Ned6502::CMP, 5},  // 0xD1
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0xD2
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0xD3
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0xD4
      {"CMP", ZERO_PAGE_X, &Ned6502::CMP, 4}, //-0xD5
      {"DEC", ZERO_PAGE_X, &Ned6502::DEC, 6}, // 0xD6
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0xD7
      {"CLD", IMPLIED, &Ned6502::CLD, 2},     //-0xD8
      {"CMP", ABSOLUTE_Y, &Ned6502::CMP, 4},  // 0xD9
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0xDA
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0xDB
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0xDC
      {"CMP", ABSOLUTE_X, &Ned6502::CMP, 4},  // 0xDD
      {"DEC", ABSOLUTE_X, &Ned6502::DEC, 7},  // 0xDE
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0xDF

      //-0xE0

      {"CPX", IMMEDIATE, &Ned6502::CPX, 2},  //-0XE0
      {"SBC", INDIRECT_X, &Ned6502::SBC, 6}, // 0xE1
      {"XXX", IMPLIED, &Ned6502::XXX},       // 0xE2
      {"XXX", IMPLIED, &Ned6502::XXX},       // 0xE3
      {"CPX", ZERO_PAGE, &Ned6502::CPX, 3},  //-0xE4
      {"SBC", ZERO_PAGE, &Ned6502::SBC, 3},  // 0xE5
      {"INC", ZERO_PAGE, &Ned6502::INC, 5},  // 0xE6
      {"XXX", IMPLIED, &Ned6502::XXX},       // 0xE7
      {"INX", IMPLIED, &Ned6502::INX, 2},    //-0xE8
      {"SBC", IMMEDIATE, &Ned6502::SBC, 2},  // 0xE9
      {"NOP", IMPLIED, &Ned6502::NOP, 2},    // 0xEA
      {"XXX", IMPLIED, &Ned6502::XXX},       // 0xEB
      {"CPX", ABSOLUTE, &Ned6502::CPX, 4},   // 0xEC
      {"SBC", ABSOLUTE, &Ned6502::SBC, 4},   // 0xED
      {"INC", ABSOLUTE, &Ned6502::INC, 6},   // 0xEE
      {"XXX", IMPLIED, &Ned6502::XXX},       // 0xEF

      // 0xF0

      {"BEQ", RELATIVE, &Ned6502::BEQ, 2},    // 0xF0
      {"SBC", INDIRECT_Y, &Ned6502::SBC, 5},  // 0xF1
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0xF2
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0xF3
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0xF4
      {"SBC", ZERO_PAGE_X, &Ned6502::SBC, 4}, // 0xF5
      {"INC", ZERO_PAGE_X, &Ned6502::INC, 6}, // 0xF6
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0xF7
      {"SED", IMPLIED, &Ned6502::SED, 2},     // 0xF8
      {"SBC", ABSOLUTE_Y, &Ned6502::SBC, 4},  // 0xF9
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0xFA
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0xFB
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0xFC
      {"SBC", ABSOLUTE_X, &Ned6502::SBC, 4},  // 0xFD
      {"INC", ABSOLUTE_X, &Ned6502::INC, 7},  // 0xFE
      {"XXX", IMPLIED, &Ned6502::XXX},        // 0xFF

  };
};

} // namespace NedNes
