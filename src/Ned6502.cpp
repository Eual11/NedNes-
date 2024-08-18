#include "../include/Ned6502.h"
#define _CRT_SECURE_NO_WARNINGS
#include <stdint.h>
#include <string>
using namespace NedNes;
bool Ned6502::getFlag(Ned6502::NedCPUFlags f) {
  return ((status & f) > 0) ? 1 : 0;
}
void NedNes::Ned6502::setFlag(Ned6502::NedCPUFlags f, bool v) {
  if (v)
    status |= f;
  else
    status &= ~f;
}

NedNes::Ned6502::Ned6502(std::shared_ptr<NedBus> _bus) { bus = _bus; }
uint8_t Ned6502::read(uint16_t addr) { return bus->cpuRead(addr); }
void Ned6502::write(uint16_t addr, uint8_t val) { bus->cpuWrite(addr, val); }
void Ned6502::connectBus(std::shared_ptr<NedBus> _bus) { bus = _bus; }

uint8_t Ned6502::fetch(uint16_t addr) {
  if (opcodes[opcode].addr_mode != IMPLIED &&
      opcodes[opcode].addr_mode != ACCUMULATOR) {
    fetched = read(addr);
  }
  return fetched;
}

void Ned6502::clock() {

  if (cycles == 0) {
    opcode = read(PC++);

    INSTRUCTION cur_opcode = opcodes[opcode];

    cycles = cur_opcode.cycles;
#ifdef LOGMODE
    fprintf(
        logFile,
        "%04X  %02X %02X %02X  %s $%04X                       A:%02X X:%02X "
        "Y:%02X P:%02X SP:%02X PPU:%4d,%2d CYC:%lld\n",
        PC - 1, opcode, 0x00, 0x00, cur_opcode.name.c_str(), PC - 1, A, X, Y,
        status, STKP, 0x00, 0x00, total_cycles + 7);
#endif

    // addressing mode
    uint8_t additional_clock1 = (this->*(cur_opcode.addr_mode))();

    // excutte instruction
    uint8_t additional_clock2 = (this->*(cur_opcode.operate))();

    cycles += additional_clock1;
    cycles += additional_clock2;
    total_cycles += cycles;
  }
  cycles--;
}

Ned6502::addr_mode_type Ned6502::curAddressingMode() {
  return opcodes[opcode].addr_mode;
}
// Implied Addressing mode there is no operand
uint8_t Ned6502::IMP() {
  fetched = A;
  return 0;
}
// Accumulator Addressing Mode: the accumulator is the operand
uint8_t Ned6502::ACC() {
  fetched = A;
  return 0x00;
}
// Immediate Addressing Mode: the operand is supplied in the instruction
uint8_t Ned6502::IMM() {

  absolute_addr = PC++;
  return 0x00;
}
// Zero Page Addressing Mode: the operand is a memory address in the Zero page
// memory ($0000-00FF)
uint8_t Ned6502::ZP() {
  absolute_addr = read(PC++);
  absolute_addr &= 0x00FF;
  return 0x00;
}
// Zero Page X Indexed, the effective address is calculated by
// adding the zero page address to the X index register (It
// wraps around zero page during overflow)
uint8_t Ned6502::ZPX() {
  absolute_addr = read(PC++) + X;
  absolute_addr &= 0x00FF;

  return 0x00;
}
// Zero Page Y indexed, same us ZPX but uses the Y index register
uint8_t Ned6502::ZPY() {
  absolute_addr = read(PC++) + Y;
  absolute_addr &= 0x00FF;
  return 0x00;
}
// Absolute Addressing Mode, the operand is the full 16 bit address
uint8_t Ned6502::ABS() {
  uint8_t lo = read(PC++);
  uint8_t hi = read(PC++);

  absolute_addr = ((hi << 8) | lo);
  return 0x00;
}
// X Indexed Absolute Addressing Mode: the effective address is
// calculated by adding the Absolute address and the value in
// the X register

uint8_t Ned6502::ABX() {
  uint16_t lo = read(PC++); // Read low byte of address
  uint16_t hi = read(PC++); // Read high byte of address

  uint16_t base_addr = (hi << 8) | lo; // Combine to form the base address
  absolute_addr = base_addr + X; // Add the X register for the final address
  absolute_addr &= 0xFFFF;       // Ensure it stays within 16-bit address space

  // Determine if a page boundary was crossed
  return (base_addr & 0xFF00) != (absolute_addr & 0xFF00);
}

// Y indexed Absolute Addressing Mode: Similar to ABX but used
// the Y register instead
uint8_t Ned6502::ABY() {
  uint8_t lo = read(PC++);
  uint8_t hi = read(PC++);

  absolute_addr = ((hi << 8) | lo) + Y;
  absolute_addr &= 0xFFFF;
  return ((absolute_addr >> 8) & 0xFF) != ((hi) & 0xFF);
}
// Indirect Addressing Mode, the operand is a pointer to where
// the data is located

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
// Indirect X index Addressing the the operand is found in the memory
// address we get by adding the zero page address and X register

uint8_t Ned6502::IZX() {

  uint16_t offset = read(PC++);
  uint16_t base = (offset + X) & 0x00FF;

  uint16_t lOfset = read(base & 0x00FF);
  uint16_t hOfset = read((base + 1) & 0x00FF);

  absolute_addr = ((hOfset << 8) | (lOfset));

  return 0x00;
}

// Indirect Y Indexed Addressing Mode, Similar to IZX but used
// the y register

uint8_t Ned6502::IZY() {
  uint16_t t = read(PC++);

  uint16_t hi = read((t + 1) & 0x00FF);
  uint16_t lo = read(t & 0x00FF);

  absolute_addr = (hi << 8) | lo;
  absolute_addr += Y;

  return (hi << 8) != (absolute_addr & 0xFF00);
}
// Relative Addressing Mode: The second operand is an offset
// value from -128 to 127 that will be added to the PC, this is
// used by branching instruction

uint8_t Ned6502::REL() {
  rel_addr = read(PC++);
  if (rel_addr & 0x80) {
    rel_addr |= 0xFF00;
  }
  return 0x00;
}

// Functionalities
//

void Ned6502::reset() {
  // Setting SP

  status = 0x00;
  STKP = 0xFD;
  setFlag(Ned6502::I, true);
  setFlag(Ned6502::D, false);
  setFlag(Ned6502::U, true);

  // Reading from the rest vector
  PC = (read(0xFFFD) << 8) | read(0xFFFC);
  A = 0x00; // accumulator
  X = 0x00; // X index register
  Y = 0x00; // Y index register

  absolute_addr = 0x00;
  fetched = 0x00;
  rel_addr = 0x00;

  // reset takes 8 clock cycles and this accounts for it
  cycles = 8;
}

// interrupt request
void Ned6502::irq() {

  if (getFlag(Ned6502::I) == 0) {
    uint16_t offset = 0x0100;
    write(STKP-- + offset, (PC >> 8) & 0xFF); // pushing the hi byte
    write(STKP-- + offset, (PC & 0xFF));      // pushing the lo byte
    write(STKP-- + offset, status);           // pushing the status flag
    setFlag(Ned6502::I, true);
    setFlag(Ned6502::U, true);
    setFlag(Ned6502::B, false);

    // reading from the reset interrupt vector

    PC = (read(0xFFFF) << 8) | (read(0xFFFE));

    // interrupt request takes 7 clock cycles and this accounts for it
    cycles = 7;
  }
}

// Non Maskable Interrupt
void Ned6502::nmi() {
  uint16_t offset = 0x0100;
  write(STKP-- + offset, (PC >> 8) & 0xFF); // pushing the hi byte
  write(STKP-- + offset, PC & 0xFF);        // pushing the lo byte
  write(STKP-- + offset, status);           // status flag

  setFlag(Ned6502::I, true);
  setFlag(Ned6502::U, true);
  setFlag(Ned6502::B, false);

  // jumpting to interrupt vector
  PC = (read(0xFFFB) << 8) | (read(0xFFFA));

  cycles = 8;
}

// INSTRUCTIONS

uint8_t Ned6502::XXX() {
  fprintf(stderr, "Not Implemented\n");
  return 0x00;
}

// Load Instructions

// Load the Accumulator

uint8_t Ned6502::LDA() {
  A = fetch(absolute_addr);
  setFlag(Z, A == 0x00);
  setFlag(N, nthBit(A, 7));
  return 0x00;
}
// Load the X register
uint8_t Ned6502::LDX() {
  X = fetch(absolute_addr);

  setFlag(Z, X == 0x00);
  setFlag(N, nthBit(X, 7));

  return 0x00;
}

// Load the Y register
uint8_t Ned6502::LDY() {
  Y = fetch(absolute_addr);

  setFlag(Z, Y == 0x00);
  setFlag(N, nthBit(Y, 7));

  return 0x00;
}

// Store Instructions

// Store the Content of Accumulator
uint8_t Ned6502::STA() {
  write(absolute_addr, A);
  return 0x00;
}

// Store the contnet of the X register
uint8_t Ned6502::STX() {
  write(absolute_addr, X);
  return 0x00;
}

// Store the content of Y register
uint8_t Ned6502::STY() {
  write(absolute_addr, Y);
  return 0x00;
}

// Register Transfer Instructions

// Transfer Accumulator to X register
uint8_t Ned6502::TAX() {
  X = A;
  setFlag(Ned6502::Z, X == 0x00);
  setFlag(Ned6502::N, nthBit(X, 7));
  return 0x00;
}

// Transfer X register to Accumulator
uint8_t Ned6502::TXA() {

  A = X;
  setFlag(Ned6502::Z, A == 0x00);
  setFlag(Ned6502::N, nthBit(A, 7));
  return 0x00;
}

// Transfer Accumulator to Y register
uint8_t Ned6502::TAY() {
  Y = A;
  setFlag(NedCPUFlags::Z, Y == 0x00);
  setFlag(NedCPUFlags::N, nthBit(Y, 7));
  return 0x00;
}

// Transfer Y register to A register
uint8_t Ned6502::TYA() {
  A = Y;
  setFlag(Ned6502::Z, A == 0x00);
  setFlag(Ned6502::N, nthBit(A, 7));

  return 0x00;
}

// Stack Operations

// Transfer the Stack Pointer to X register

uint8_t Ned6502::TSX() {
  X = STKP;

  setFlag(NedCPUFlags::Z, X == 0x00);
  setFlag(NedCPUFlags::N, nthBit(X, 7));
  return 0x00;
}

// Transfer X register to Stack Pointer

uint8_t Ned6502::TXS() {
  STKP = X;
  return 0x00;
}

// Push Accumulator to the stack
uint8_t Ned6502::PHA() {
  uint16_t offset = 0x0100;
  write(STKP + offset, A);
  STKP--;
  return 0x00;
}

// Push Processor Statys to the stack

uint8_t Ned6502::PHP() {
  uint16_t offset = 0x0100;
  /* setFlag(NedCPUFlags::B, true); */
  /* setFlag(NedCPUFlags::U, true); */
  write(offset + STKP, status | B | U);
  /* setFlag(NedCPUFlags::B, false); */
  /* setFlag(NedCPUFlags::U, false); */

  STKP--;

  return 0x00;
}

// Pull Accumulator from the stack

uint8_t Ned6502::PLA() {
  uint16_t offset = 0x0100;
  A = read(++STKP + offset);
  setFlag(NedCPUFlags::Z, A == 0x00);
  setFlag(NedCPUFlags::N, nthBit(A, 7));
  return 0x00;
}

// Pull Processor Status from the stack
uint8_t Ned6502::PLP() {
  uint16_t offset = 0x0100;
  status = read(++STKP + offset);
  setFlag(NedCPUFlags::U, true);
  setFlag(NedCPUFlags::B, false);
  return 0x00;
}

// Logic Instructions

// Logical And
uint8_t Ned6502::AND() {
  fetched = fetch(absolute_addr);

  A = A & fetched;
  setFlag(NedCPUFlags::Z, A == 0);
  setFlag(NedCPUFlags::N, nthBit(A, 7));
  return 0x00;
}

// Bit testing
uint8_t Ned6502::BIT() {
  fetched = fetch(absolute_addr);
  setFlag(NedCPUFlags::N, nthBit(fetched, 7));
  setFlag(NedCPUFlags::V, nthBit(fetched, 6));
  setFlag(NedCPUFlags::Z, (fetched & A) == 0);
  return 0x00;
}

// Exclusive Or

uint8_t Ned6502::EOR() {
  fetched = fetch(absolute_addr);
  A = A ^ fetched;
  setFlag(NedCPUFlags::Z, A == 0);
  setFlag(NedCPUFlags::N, nthBit(A, 7));
  return 0x00;
}
uint8_t Ned6502::ORA() {
  fetched = fetch(absolute_addr);
  A = A | fetched;
  setFlag(NedCPUFlags::Z, A == 0);
  setFlag(NedCPUFlags::N, nthBit(A, 7));

  return 0x00;
}

// Arithmetic Operations

// Add with carry
uint8_t Ned6502::ADC() {

  fetched = fetch(absolute_addr);
  uint16_t sum =
      (uint16_t)A + (uint16_t)fetched + (uint16_t)getFlag(NedCPUFlags::C);
  setFlag(NedCPUFlags::C, sum > 0xFF);
  setFlag(NedCPUFlags::Z, (sum & 0x00FF) == 0x00);
  setFlag(NedCPUFlags::N, nthBit(sum, 7));
  setFlag(NedCPUFlags::V, ((~((uint16_t)A ^ (uint16_t)fetched)) &
                           ((uint16_t)A ^ (uint16_t)sum)) &
                              0x0080);

  A = sum & 0x00FF;
  return 0x00;
}

// Substract with carry

uint8_t Ned6502::SBC() {
  fetched = fetch(absolute_addr) ^ 0x00FF;

  uint16_t temp =
      (uint16_t)A + (uint16_t)fetched + (uint16_t)getFlag(NedCPUFlags::C);
  setFlag(NedCPUFlags::C, temp > 0xFF);
  setFlag(NedCPUFlags::V, ((temp ^ (uint16_t)A) & (temp ^ fetched)) & 0x0080);
  setFlag(NedCPUFlags::N, temp & 0x0080);
  setFlag(NedCPUFlags::Z, (temp & 0x00FF) == 0);

  A = temp & 0x00FF;

  return 0x00;
}

// Compare Memory and Accumulator

uint8_t Ned6502::CMP() {
  fetched = fetch(absolute_addr);
  // WARNING: this is ambiguous
  uint16_t res = A - fetched;
  setFlag(NedCPUFlags::Z, (res & 0x00FF) == 0x00);
  setFlag(NedCPUFlags::N, nthBit(res, 7));
  setFlag(NedCPUFlags::C, A >= fetched);
  return 0x00;
}

// Compare X register to memory

uint8_t Ned6502::CPX() {
  fetched = fetch(absolute_addr);
  uint16_t res = X - fetched;
  setFlag(NedCPUFlags::C, X >= fetched);
  setFlag(NedCPUFlags::N, nthBit(res, 7));
  setFlag(NedCPUFlags::Z, (res & 0x00FF) == 0);
  return 0x00;
}

// Compare Y register to Memory

uint8_t Ned6502::CPY() {
  fetched = fetch(absolute_addr);
  uint16_t res = Y - fetched;
  setFlag(NedCPUFlags::C, Y >= fetched);
  setFlag(NedCPUFlags::N, nthBit(res, 7));
  setFlag(NedCPUFlags::Z, (res & 0x00FF) == 0);
  return 0x00;
}

// Increament and Decreament Instructions
//
//
//
// Decreament Memory by 1
uint8_t Ned6502::DEC() {

  fetched = fetch(absolute_addr);
  fetched--;
  setFlag(NedCPUFlags::Z, fetched == 0);
  setFlag(NedCPUFlags::N, nthBit(fetched, 7));
  write(absolute_addr, fetched);
  return 0x00;
}

// Decreament X register
uint8_t Ned6502::DEX() {
  X--;
  setFlag(Z, X == 0x00);
  setFlag(N, nthBit(X, 7));
  return 0x00;
}
// Decreament Y register
uint8_t Ned6502::DEY() {
  Y--;
  setFlag(Z, Y == 0x00);
  setFlag(N, nthBit(Y, 7));
  return 0x00;
}

// Increament Instructions

uint8_t Ned6502::INC() {

  fetched = fetch(absolute_addr);
  fetched++;
  setFlag(NedCPUFlags::Z, fetched == 0);
  setFlag(NedCPUFlags::N, nthBit(fetched, 7));
  write(absolute_addr, fetched);
  return 0x00;
}
uint8_t Ned6502::INX() {
  X++;
  setFlag(Z, X == 0x00);
  setFlag(N, nthBit(X, 7));
  return 0x00;
}
uint8_t Ned6502::INY() {
  Y++;
  setFlag(Z, Y == 0x00);
  setFlag(N, nthBit(Y, 7));
  return 0x00;
}

// Status Flag Changes
//
//
// Clear Carry Flag
uint8_t Ned6502::CLC() {

  setFlag(NedCPUFlags::C, false);
  return 0x00;
}

// Clear Decimal Mode  Flag
uint8_t Ned6502::CLD() {

  setFlag(NedCPUFlags::D, false);
  return 0x00;
}
// Clear Interrupt Flag
uint8_t Ned6502::CLI() {

  setFlag(NedCPUFlags::I, false);
  return 0x00;
}
// Clear overflow Flag
uint8_t Ned6502::CLV() {

  setFlag(NedCPUFlags::V, false);
  return 0x00;
}
uint8_t Ned6502::SEC() {
  setFlag(NedCPUFlags::C, true);
  return 0x00;
}
uint8_t Ned6502::SED() {
  setFlag(NedCPUFlags::D, true);
  return 0x00;
}
uint8_t Ned6502::SEI() {
  setFlag(NedCPUFlags::I, true);
  return 0x00;
}

// Shift Instructions

uint8_t Ned6502::ASL() {
  fetched = fetch(absolute_addr);
  setFlag(NedCPUFlags::C, nthBit(fetched, 7));
  fetched = fetched << 1;
  setFlag(NedCPUFlags::Z, fetched == 0x00);
  setFlag(NedCPUFlags::N, nthBit(fetched, 7));
  if (curAddressingMode() == ACCUMULATOR)
    A = fetched;
  else
    write(absolute_addr, fetched);
  return 0x00;
}
uint8_t Ned6502::LSR() {
  fetched = fetch(absolute_addr);
  setFlag(NedCPUFlags::C, nthBit(fetched, 0));
  fetched = fetched >> 1;
  setFlag(NedCPUFlags::Z, fetched == 0x00);
  setFlag(NedCPUFlags::N, false);
  if (curAddressingMode() == ACCUMULATOR)
    A = fetched;
  else
    write(absolute_addr, fetched);
  return 0x00;
}
uint8_t Ned6502::ROL() {
  fetched = fetch(absolute_addr);
  uint8_t tmp_fetched = fetched;
  fetched = fetched << 1;
  uint8_t c = getFlag(NedCPUFlags::C);
  fetched |= (uint8_t)(c);
  setFlag(NedCPUFlags::C, nthBit(tmp_fetched, 7));
  setFlag(NedCPUFlags::Z, fetched == 0x00);
  setFlag(NedCPUFlags::N, nthBit(fetched, 7));
  if (curAddressingMode() == ACCUMULATOR)
    A = fetched;
  else
    write(absolute_addr, fetched);
  return 0x00;
}

uint8_t Ned6502::ROR() {
  fetched = fetch(absolute_addr);
  uint8_t tmp_fetched = fetched;
  fetched = fetched >> 1;
  uint8_t c = getFlag(NedCPUFlags::C);

  fetched |= (((uint8_t)c) << 7);
  setFlag(NedCPUFlags::C, nthBit(tmp_fetched, 0));
  setFlag(NedCPUFlags::Z, fetched == 0x00);
  setFlag(NedCPUFlags::N, nthBit(fetched, 7));
  if (curAddressingMode() == ACCUMULATOR)
    A = fetched;
  else
    write(absolute_addr, fetched);
  return 0x00;
}

// Jump and Calls
//

// Jump to location
uint8_t Ned6502::JMP() {
  PC = absolute_addr;
  return 0x00;
}
uint8_t Ned6502::JSR() {
  uint16_t offset = 0x0100;
  uint16_t return_address = PC - 1;
  // writing low byte
  write(offset + STKP--, (return_address >> 8) & 0x00FF);
  // writing high pyte
  write(offset + STKP--, return_address & 0x00FF);
  PC = absolute_addr;
  return 0x00;
}

uint8_t Ned6502::RTS() {
  uint16_t offset = 0x0100;
  STKP++;
  uint8_t lo = read(STKP + offset);
  STKP++;
  uint8_t hi = read(STKP + offset);
  PC = ((hi << 8) | lo) + 1;
  return 0x00;
}

// Branching Instructions
//

// Branch on Carry Clear
uint8_t Ned6502::BCC() {
  if (!getFlag(NedCPUFlags::C)) {
    int16_t new_addr = (int16_t)PC + rel_addr;
    PC = new_addr;
    return 0x01;
  }
  return 0x00;
}
// Branch on Carry Set
uint8_t Ned6502::BCS() {
  if (getFlag(NedCPUFlags::C)) {
    int16_t new_addr = (int16_t)PC + rel_addr;
    PC = new_addr;
    return 0x01;
  }
  return 0x00;
}
// Branch on Result Zero

uint8_t Ned6502::BEQ() {
  if (getFlag(NedCPUFlags::Z)) {
    int16_t new_addr = (int16_t)PC + rel_addr;
    PC = new_addr;
    return 0x01;
  }
  return 0x00;
}
uint8_t Ned6502::BMI() {
  if (getFlag(NedCPUFlags::N)) {
    int16_t new_addr = (int16_t)PC + rel_addr;
    PC = new_addr;
    return 0x01;
  }
  return 0x00;
}
uint8_t Ned6502::BNE() {
  if (!getFlag(NedCPUFlags::Z)) {
    int16_t new_addr = (int16_t)PC + rel_addr;
    PC = new_addr;
    return 0x01;
  }
  return 0x00;
}
uint8_t Ned6502::BPL() {
  if (!getFlag(NedCPUFlags::N)) {
    int16_t new_addr = (int16_t)PC + rel_addr;
    PC = new_addr;
    return 0x01;
  }
  return 0x00;
}
uint8_t Ned6502::BVC() {

  if (!getFlag(NedCPUFlags::V)) {
    int16_t new_addr = (int16_t)PC + rel_addr;
    PC = new_addr;
    return 0x01;
  }
  return 0x00;
}
uint8_t Ned6502::BVS() {

  if (getFlag(NedCPUFlags::V)) {
    int16_t new_addr = (int16_t)PC + rel_addr;
    PC = new_addr;
    return 0x01;
  }
  return 0x00;
}

// System Functions

uint8_t Ned6502::NOP() { return 0x00; }

uint8_t Ned6502::RTI() {
  uint16_t offset = 0x0100;
  status = read(++STKP + offset);
  setFlag(NedCPUFlags::U, true);
  setFlag(NedCPUFlags::B, false);
  uint8_t lo = read(++STKP + offset);
  uint8_t hi = read(++STKP + offset);
  PC = ((hi << 8) | lo);

  return 0x00;
}

uint8_t Ned6502::BRK() {
  uint16_t offset = 0x0100;
  uint16_t return_address = ++PC;
  // writing low byte
  write(offset + STKP--, (return_address >> 8) & 0x00FF);
  // writing high pyte
  write(offset + STKP--, return_address & 0x00FF);

  setFlag(NedCPUFlags::B, true);
  /* setFlag(NedCPUFlags::U, true); */

  write(offset + STKP--, status);

  uint8_t lo = read(0xFFFE);
  uint8_t hi = read(0xFFFE);
  setFlag(NedCPUFlags::B, false);
  setFlag(NedCPUFlags::I, true);
  PC = (hi << 8) | lo;
  return 0x00;
}
#ifdef LOGMODE
void Ned6502::logCpuState() {
  fprintf(logFile,
          "%04X  %02X %02X %02X  %s $%04X                       A:%02X X:%02X "
          "Y:%02X P:%02X SP:%02X PPU:%4d,%2d CYC:%lld\n",
          PC, opcode, 0x00, 0x00, "instr", PC - 1, A, X, Y, status, STKP, 0x00,
          0x00, total_cycles);
}
#endif
std::map<uint16_t, std::string> Ned6502::disassemble(uint16_t count) {

  // getting the current opcode fro the program counter

  // $8000: A9 01     LDA #$01
  std::map<uint16_t, std::string> disassembled;
  uint16_t tmp_PC = PC;

  for (int i = 0; i < count; i++) {
    // fetch N Instructions

    uint16_t opcode_addr = tmp_PC;
    uint8_t opcode = read(tmp_PC++);
    INSTRUCTION cur_opcode = opcodes[opcode];

    uint8_t (Ned6502::*addr_mode)() = cur_opcode.addr_mode;

    std::string mnemonic = cur_opcode.name;
    char buffer[64] = {'\0'};
    if (addr_mode == &Ned6502::IMP) {
      // Implied Instruction
      sprintf(buffer, "%X   %s ", opcode, cur_opcode.name.c_str());
    }

    else if (addr_mode == &Ned6502::IMM) {
      // Immediate Mode Instruction
      uint8_t data = read(tmp_PC++);
      sprintf(buffer, "%X %X   %s #$%X", opcode, data, mnemonic.c_str(), data);
    } else if (addr_mode == &Ned6502::ACC) {
      // Accumulator Addressing Mode
      // Syntax: ASL A
      sprintf(buffer, "%X   %s A", opcode, mnemonic.c_str());
    } else if (addr_mode == &Ned6502::ZP) {
      // Zero Page Addressing Mode Instruction
      uint16_t addr = read(tmp_PC++);
      addr &= 0x00FF;
      sprintf(buffer, "%X %X   %s $%X", opcode, addr, mnemonic.c_str(), addr);
    } else if (addr_mode == &Ned6502::ZPX) {
      // Zero Page X Indexed
      // Syntax: LDA $10,X
      uint16_t addr = read(tmp_PC++);
      addr &= 0x00FF;
      sprintf(buffer, "%X %X   %s $%X, X", opcode, addr, mnemonic.c_str(),
              addr);
    } else if (addr_mode == &Ned6502::ZPY) {
      // Zero Page Y Indexed
      // Syntax: LDA $10,Y
      uint16_t addr = read(tmp_PC++);
      addr &= 0x00FF;
      sprintf(buffer, "%X %X   %s $%X, Y", opcode, addr, mnemonic.c_str(),
              addr);
    } else if (addr_mode == &Ned6502::ABS) {
      // Absolute Addressing Mode
      // Syntax: LDA $1000
      uint8_t lo = read(tmp_PC++);
      uint8_t hi = read(tmp_PC++);

      uint16_t abs_addr = ((hi << 8) | lo);
      sprintf(buffer, "%X %X %X   %s $%X", opcode, lo, hi, mnemonic.c_str(),
              abs_addr);
    } else if (addr_mode == &Ned6502::ABX) {
      // Absolute X Indexed Addressing mode
      // Syntax: LDA $1000, X
      uint8_t lo = read(tmp_PC++);
      uint8_t hi = read(tmp_PC++);
      uint16_t abs_addr = ((hi << 8) | lo);
      sprintf(buffer, "%X %X %X   %s $%X, X", opcode, lo, hi, mnemonic.c_str(),
              abs_addr);
    } else if (addr_mode == &Ned6502::ABY) {
      // Absolute Y Indexed Addressing mode
      // Syntax: LDA $1000, Y
      uint8_t lo = read(tmp_PC++);
      uint8_t hi = read(tmp_PC++);
      uint16_t abs_addr = ((hi << 8) | lo);
      sprintf(buffer, "%X %X %X   %s $%X, Y", opcode, lo, hi, mnemonic.c_str(),
              abs_addr);
    } else if (addr_mode == &Ned6502::IND) {
      // Indirect Addressing Mode
      // Syntax: JMP ($1000)
      uint8_t lo = read(tmp_PC++);
      uint8_t hi = read(tmp_PC++);

      uint16_t abs_addr = ((hi << 8) | lo);
      sprintf(buffer, "%X %X %X   %s ($%X)", opcode, lo, hi, mnemonic.c_str(),
              abs_addr);
    } else if (addr_mode == &Ned6502::IZX) {
      // Indirect X indexed addressing mode
      // Syntax: LDA ($10, X)

      uint8_t abs_addr = read(tmp_PC++) & 0x00FF;
      sprintf(buffer, "%X %X   %s ($%X, X)", opcode, abs_addr, mnemonic.c_str(),
              abs_addr);
    } else if (addr_mode == &Ned6502::IZY) {
      // Indirect Y indexed addressing mode
      // Syntax: LDA ($10), Y

      uint8_t abs_addr = read(tmp_PC++) & 0x00FF;
      sprintf(buffer, "%X %X   %s ($%X), Y", opcode, abs_addr, mnemonic.c_str(),
              abs_addr);
    } else if (addr_mode == &Ned6502::REL) {
      // Relateive Addressing Mode
      // Syntax: BEQ Label
      int8_t r_addr = read(tmp_PC++);
      if (r_addr & 0x80) {
        r_addr |= 0xFF00;
      }
      int16_t new_addr = (int16_t)opcode_addr + r_addr;
      sprintf(buffer, "%X %X   %s %X", opcode, r_addr, mnemonic.c_str(),
              (uint16_t)new_addr);
    }

    disassembled[opcode_addr] = std::string(buffer);
  }
  return disassembled;
}
bool Ned6502::complete() { return cycles == 0; }
