#include "../include/NedManager.h"

int main(int argc, char **argv) {
  NedNes::NedManager Manager;
  Manager.Init();
  Manager.SetupAudio();
  Manager.Run();
  return 0;
}
