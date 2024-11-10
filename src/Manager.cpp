#include "../include/NedManager.h"

int main(int argc, char **argv) {
  NedNes::NedManager Manager;
  Manager.Init();
  Manager.Run();
  return 0;
}
