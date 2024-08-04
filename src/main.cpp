#include "../include/NedNes.h"

int main(void) {
  NedNes::NedCartrdige cart("../rom/tests/nes-test-roms/tutor/tutor.nes");
  if (cart.imageValid()) {
    printf("Rom Loaded\n");
  }
  return 0;
}
