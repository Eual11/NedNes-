#include "../include/NedJoypad.h"

using namespace NedNes;
void NedJoypad::Press(JOYPAD_BUTTONS btn) { state |= 0x1 << (uint8_t)(btn); };
void NedJoypad::Release(JOYPAD_BUTTONS btn) {
  state &= (0xFF ^ (0x1 << (uint8_t)(btn)));
};
void NedJoypad::write(uint8_t strobe_state) { strobe = strobe_state & 0x01; }

bool NedJoypad::read() {
  if (strobe) {

    // if the stobe is on, onlyr return the state of A
    return state & 0x01;
  }

  else if (idx <= 7) {
    bool cur_state = (state >> (idx)) & 0x01;
    idx += 1;
    if (idx > 7) {
      idx = 0;
    }
    return cur_state;
  }
  return false;
}
void NedJoypad::setState(uint8_t st) { this->state = st; }
