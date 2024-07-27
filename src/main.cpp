#include "../include/NedNes.h"

int main(void) {
  NedNes::NedNesEmulator emu;

  // this all is awful
  FILE *log = fopen("../rom/tests/nedlog.log", "w");

  if (!log) {
    fprintf(stderr, "Failed to open log file");
    return -1;
  }
  emu.nedCpu.reset();
  emu.nedCpu.logFile = log;
  emu.nedCpu.PC = 0x8000;
  emu.nedCpu.PC = 0xC000;
  emu.nedCpu.cycles = 7;
  emu.loadRom("../rom/tests/nestest.nes", emu.nedCpu.PC);

  while (!emu.nedCpu.complete) {
    emu.nedCpu.clock();
  }
  printf("Job COmpleted\n");

  return 0;
}
