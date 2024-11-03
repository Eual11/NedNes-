#include "../include/NedBus.h"
#include <cstdio>
#define _CRT_SECURE_NO_WARNINGS
#include <cstdint>
#include <memory>

// NedBus Constructor, currently only just filling the ram with 0s
NedNes::NedBus::NedBus() { std::fill(std::begin(ram), std::end(ram), 0x00); }

// Reading from memory, the NedBus is 16 bit wide bus so it is capable of
// addressing 64Kb of memory

uint8_t NedNes::NedBus::cpuRead(uint16_t addr) {

  uint8_t data = 0x00;
  if (cart->cpuRead(addr, data)) {
  } else if (addr >= 0x0000 && addr <= 0x1FFF) {
    // mirroring the cpu read
    data = ram[addr & 0x7FF];
  } else if (addr >= 0x2000 && addr <= 0x3FFF) {
    data = ppu->cpuRead(addr & 0x2007);
  } else if (addr == 0x4016 || addr == 0x4017) {

    auto pad = joypads[addr - 0x4016];
    if (pad) {
      data = pad->read();
    }
  } else if (addr >= 0x4000 && addr <= 0x4013 || addr == 0x4015 ||
             addr == 0x4017) {
    data = apu->cpuRead(addr);
  }

  return data;
}
// Writing to the bus, currently we only have the ram so we write to it
void NedNes::NedBus::cpuWrite(uint16_t addr, uint8_t val) {

  if (cart->cpuWrite(addr, val)) {
  } else if (addr >= 0x0000 && addr <= 0x1FFF) {
    // mirroring the cpu write
    ram[addr & 0x7FF] = val;
  } else if (addr >= 0x2000 && addr <= 0x3FFF) {
    ppu->cpuWrite(addr & 0x2007, val);
  } else if (addr == 0x4014) {

    // writing to the oamdma register initiates DMA
    dma_transfer = true;
    dma_page = val;
    dma_addr = ppu->getOamAddr();
    dma_transfered_data = 0;
  } else if (addr == 0x4016 || addr == 0x4017) {
    auto pad = joypads[addr - 0x4016];

    if (pad) {
      pad->write(val);
    }
  } else if (addr >= 0x4000 && addr <= 0x4013 || addr == 0x4015 ||
             addr == 0x4017) {
    if (apu) {
      apu->cpuWrite(addr, val);
    }
  }
}

// connecting a cpu to the bus
void NedNes::NedBus::connectCpu(std::shared_ptr<Ned6502> _cpu) { cpu = _cpu; }
void NedNes::NedBus::connectPpu(std::shared_ptr<Ned2C02> _ppu) { ppu = _ppu; }
void NedNes::NedBus::connectCartridge(std::shared_ptr<NedCartrdige> _cart) {

  this->cart = _cart;
}
void NedNes::NedBus::clock() {
  if (ppu) {
    ppu->clock();
  }
  /* if (apu) { */
  /*   apu->clock(); */
  /* } */
  if (SystemClock % 3 == 0) {

    // if there is dma transfer suspend the cpu clock
    if (dma_transfer) {

      if (dma_pre) {
        // the dma is synchronized with the cpu clock so it takes a cycle to
        // start reading
        if (SystemClock % 2 == 1)
          dma_pre = false;
      } else {

        // we read data from memeory during even cycles
        if (SystemClock % 2 == 0) {
          dma_data = cpuRead(((uint16_t)(dma_page) << 8) | dma_addr);

        } else {
          // write them on odd cycles
          ppu->pOAM[dma_addr] = dma_data;
          dma_addr = (dma_addr + 1) % 256;
          dma_transfered_data += 1;
          dma_data &= 0xFF;
          if (dma_transfered_data == 256) {
            dma_pre = true;
            dma_transfer = false;
            dma_transfered_data = 0;
          }
        }
      }

    } else {
      // if not, go on cpu, RUN
      cpu->clock();
    }
  }
  if (ppu->nmi) {
    /* printf("Non Maskable intrupt\n"); */
    ppu->PPUSTATUS.bits.vblank = 0x00; // clearing vblank
    ppu->nmi = false;
    cpu->nmi();
  }

  if (cart->imageValid() && cart->getMapper()->irqState()) {
    cart->getMapper()->irqClear();
    cpu->irq();
  }
  SystemClock++;
}
void NedNes::NedBus::Press(int n, JOYPAD_BUTTONS btn) {
  n %= 2;

  auto pad = joypads[n];
  if (pad) {
    pad->Press(btn);
  }
}
void NedNes::NedBus::setState(int n, uint8_t state) {
  n %= 2;

  auto pad = joypads[n];
  if (pad) {
    pad->setState(state);
  }
}
void NedNes::NedBus::Release(int n, JOYPAD_BUTTONS btn) {
  n %= 2;

  auto pad = joypads[n];
  if (pad) {
    pad->Release(btn);
  }
}

void NedNes::NedBus::reset() {
  cpu->reset();
  ppu->reset();
}
void NedNes::NedBus::connectJoypad(int n, std::shared_ptr<NedJoypad> con) {
  n %= 2;
  joypads[n] = con;
}
void NedNes::NedBus::connectApu(std::shared_ptr<Ned2A03> apu) {
  this->apu = apu;
}
