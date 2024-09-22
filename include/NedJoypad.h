
#include <cstdint>

namespace NedNes {
enum JOYPAD_BUTTONS {

  BUTTON_A,
  BUTTON_B,
  BUTTON_SELECT,
  BUTTON_START,
  BUTTON_UP,
  BUTTON_DOWN,
  BUTTON_LEFT,
  BUTTON_RIGHT
};

class NedJoypad {

public:
  NedJoypad() = default;

  // for user
  //
  void Press(JOYPAD_BUTTONS btn);
  void Release(JOYPAD_BUTTONS btn);
  void setState(uint8_t st);
  //  for cpu access
  void write(uint8_t);
  bool read();

private:
  uint8_t state = 0X00;
  bool strobe = true;
  int idx = 0;
};
} // namespace NedNes
