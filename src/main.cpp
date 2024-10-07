#include <SDL2/SDL_gamecontroller.h>
#include <SDL2/SDL_keycode.h>
#include <cstdint>
#include <string>
#define _CRT_SECURE_NO_WARNINGS
#include "../include/NedNes.h"
#include "../include/RenderUtils.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_video.h>

#include <memory>
#include <set>

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
bool bFreeRun = false;
SDL_Window *gWindow = nullptr;
int p_idx = 0;
SDL_Renderer *gRenderer = nullptr;

TTF_Font *global_font = nullptr;

// Screen and pattern table area
//
//

SDL_Rect scrArea = {0, 0, 256 * 3, 240 * 3};
SDL_Rect patternTableArea1 = {0, (WINDOW_HEIGHT - (128 + 30)), 128, 128};
SDL_Rect patternTableArea2 = {256, (WINDOW_HEIGHT - (128 + 30)), 128, 128};
SDL_Rect nametableArea = {0, 300, 256, 240};

// TODO: support for Turbo!
std::map<SDL_KeyCode, NedNes::JOYPAD_BUTTONS> keymap = {

    {SDLK_a, NedNes::BUTTON_A},       {SDLK_d, NedNes::BUTTON_B},

    {SDLK_q, NedNes::BUTTON_SELECT},  {SDLK_e, NedNes::BUTTON_START},
    {SDLK_UP, NedNes::BUTTON_UP},     {SDLK_DOWN, NedNes::BUTTON_DOWN},
    {SDLK_LEFT, NedNes::BUTTON_LEFT}, {SDLK_RIGHT, NedNes::BUTTON_RIGHT},
};
std::map<SDL_GameControllerButton, NedNes::JOYPAD_BUTTONS> joystickMap = {

    {SDL_CONTROLLER_BUTTON_A, NedNes::BUTTON_A},
    {SDL_CONTROLLER_BUTTON_B, NedNes::BUTTON_B},
    {SDL_CONTROLLER_BUTTON_BACK, NedNes::BUTTON_SELECT},
    {SDL_CONTROLLER_BUTTON_START, NedNes::BUTTON_START},

    {SDL_CONTROLLER_BUTTON_DPAD_UP, NedNes::BUTTON_UP},
    {SDL_CONTROLLER_BUTTON_DPAD_DOWN, NedNes::BUTTON_DOWN},
    {SDL_CONTROLLER_BUTTON_DPAD_LEFT, NedNes::BUTTON_LEFT},
    {SDL_CONTROLLER_BUTTON_DPAD_RIGHT, NedNes::BUTTON_RIGHT},
};
std::set<SDL_GameController *> joysticks;
SDL_GameController *mapped_joystick[2];
void init();
void HandleController(std::shared_ptr<NedNes::NedBus> bus);
uint8_t getControllerStateFromKeyboard();
uint8_t getControllerStateFromJoyStick(SDL_GameController *);
void close_program();

int main(int argc, char **argv) {

  // TODO: better controllers
  init();

  mapped_joystick[0] = nullptr;
  mapped_joystick[1] = nullptr;
  auto cart =
      std::make_shared<NedNes::NedCartrdige>("../rom/games/Contra (U).nes");

  auto joypad1 = std::make_shared<NedNes::NedJoypad>();
  auto joypad2 = std::make_shared<NedNes::NedJoypad>();
  // setting up nednes bus
  auto EmuBus = std::make_shared<NedNes::NedBus>();
  auto CPU = std::make_shared<NedNes::Ned6502>();
  auto PPU = std::make_shared<NedNes::Ned2C02>(gRenderer);
  PPU->connectBus(EmuBus);
  PPU->connectCart(cart);
  ;
  EmuBus->connectCartridge(cart);
  EmuBus->connectPpu(PPU);
  EmuBus->connectCpu(CPU);
  EmuBus->connectJoypad(0, joypad1);
  EmuBus->connectJoypad(1, joypad2);
  CPU->connectBus(EmuBus);
  /* CPU->logFile = f; */

  EmuBus->reset();
  printf("Program %s running with %d args\n", argv[0], argc);
  if (cart->imageValid()) {
    printf("Rom Loaded\n");
  }

  // fetching all conntected controllers
  for (int i = 0; i < SDL_NumJoysticks(); i++) {
    if (SDL_IsGameController((i))) {
      SDL_GameController *ctrler = SDL_GameControllerOpen(i);
      SDL_Log("controller %d added\n", SDL_GameControllerNameForIndex(i));
      if (ctrler) {
        joysticks.insert(ctrler);
      }
    }
  }
  // setting controller 2 with  controller mapped
  // NOTE: THIS IS ONLY FOR TESTING PURPUSE! and it will be removed once
  // everything is done

  bool quit = false;
  std::map<uint16_t, std::string> disMap;

  SDL_Event event;
  while (!quit) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        quit = true;
      }
      if (event.type == SDL_CONTROLLERDEVICEADDED) {

        if (SDL_IsGameController(event.cdevice.which)) {
          SDL_Log("Controller %s added\n",
                  SDL_GameControllerNameForIndex(event.cdevice.which));
          SDL_GameController *c = SDL_GameControllerOpen(event.cdevice.which);

          if (c) {
            joysticks.insert(c);
          }
          SDL_Log("%d Total controllers\n", joysticks.size());
        }
        // remapping the controlelrs

        if (joysticks.size() == 1) {
          mapped_joystick[1] = *joysticks.begin();
          SDL_Log("%s is now Player 2\n",
                  SDL_GameControllerNameForIndex(event.cdevice.which));
        }
        auto iter = joysticks.begin();
        if (joysticks.size() > 1) {
          for (int i = 0; i < 2; i++) {
            mapped_joystick[i] = *iter;
            SDL_Log("%s is now Player %d \n", SDL_GameControllerName(*iter),
                    i + 1);
            iter++;
          }
        }
      }
      if (event.type == SDL_CONTROLLERDEVICEREMOVED) {

        SDL_Log("%d controller removed\n", event.cdevice.which);

        SDL_GameController *ctrl =
            SDL_GameControllerFromInstanceID(event.cdevice.which);

        if (mapped_joystick[0] == ctrl) {
          mapped_joystick[0] = nullptr;
          SDL_Log("Player 1 is now keyboard \n");
        }
        if (mapped_joystick[1] == ctrl) {
          mapped_joystick[1] = nullptr;
          SDL_Log("Player 2 is now keyboard \n");
        }
        if (joysticks.find(ctrl) != joysticks.end()) {
          joysticks.erase(ctrl);
        }
        SDL_GameControllerClose(ctrl);

        SDL_Log("%d controller closed\n", event.cdevice.which);
        SDL_Log("%d Total controllers\n", joysticks.size());
      }

      // Get the current state of the keyboard
      /* const Uint8 *state = SDL_GetKeyboardState(NULL); */
      /**/
      /* // Initialize the joypad state to zero */
      /* uint8_t joypadState = 0; */
      /**/
      /* // Check for key presses and update joypad state */
      /* for (const auto &pair : keymap) { */
      /*   if (state[SDL_GetScancodeFromKey(pair.first)]) { */
      /*     joypadState |= (1 << pair.second); // Set the corresponding bit */
      /*   } */
      /* } */
      /* EmuBus->setState(0, joypadState); */
      /**/
      // Update the joypad state

      if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
        case SDLK_s: {
          // step a whole instruction
          //

          int clc = 0;
          do {
            clc++;
            EmuBus->clock();

          } while (!EmuBus->cpu->complete());
          do {
            clc++;
            EmuBus->clock();

          } while (EmuBus->cpu->complete());
          break;
        }
        case SDLK_RETURN: {

          printf("Framed Skipped\n");
          // step one clock
          while (!EmuBus->ppu->isFrameComplete()) {
            EmuBus->clock();
          }
          while (!EmuBus->cpu->complete()) {
            EmuBus->clock();
          }
          while (EmuBus->cpu->complete()) {
            EmuBus->clock();
          }

          break;
        }
        case SDLK_f: {
          bFreeRun = !bFreeRun;
          break;
        }
        case SDLK_p: {
          // shifting pallete
          p_idx++;
          p_idx %= 8;
          break;
        }
        }
      }
    }
    HandleController(EmuBus);
    if (bFreeRun) {
      while (!EmuBus->ppu->isFrameComplete()) {
        EmuBus->clock();
      }
      while (!EmuBus->cpu->complete()) {
        EmuBus->clock();
      }
      while (EmuBus->cpu->complete()) {
        EmuBus->clock();
      }
    }
    SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0xff, 0x00);
    SDL_RenderClear(gRenderer);

    // stuff happens

    SDL_Color col = {0xff, 0xff, 0xff, 0xff};

    SDL_Rect R = DrawCPUReg(EmuBus->cpu, gRenderer, global_font,
                            WINDOW_WIDTH - 350, 10, col);
    R.h += 40;
    //
    //
    // Drawing disassembled instructions
    disMap = EmuBus->cpu->disassemble(4);
    col = {0x04, 0x09c, 0xf4, 0xff};
    for (auto &d : disMap) {
      std::string instr = toHex(d.first) + " " + d.second;
      SDL_Rect r = DrawText(gRenderer, global_font, instr, WINDOW_WIDTH - 350,
                            R.h + 10, col);
      R.h += r.h;
      col = {0xff, 0xff, 0xff, 0x00};
    }
    NedNes::Ned2C02::oamEntry *oam =
        (NedNes::Ned2C02::oamEntry *)EmuBus->ppu->pOAM;

    auto toInt = [&](int s) { return std::to_string(s); };

    /* for (int i = 0; i < 30; i++) { */
    /*   auto sprite = oam[i]; */
    /*   std::string entry = toHex(i) + " " + "(" + toInt(sprite.x) + ", " + */
    /*                       toInt(sprite.y) + ") " + toHex(sprite.id) + " " +
     */
    /*                       toHex(sprite.attrib); */
    /*   SDL_Rect r = DrawText(gRenderer, global_font, entry, WINDOW_WIDTH -
     * 350, */
    /*                         R.h + 10, col); */
    /*   R.h += r.h; */
    /**/
    /*   col = {0xff, 0xff, 0xff, 0x00}; */
    /* } */

    SDL_RenderCopy(gRenderer, EmuBus->ppu->getScreenTexture(), nullptr,
                   &scrArea);

#ifdef _DEBUG
    SDL_RenderCopy(gRenderer, EmuBus->ppu->getPatternTable(0, p_idx), nullptr,
                   &patternTableArea1);
    SDL_RenderCopy(gRenderer, EmuBus->ppu->getPatternTable(1, p_idx), nullptr,
                   &patternTableArea2);

    for (int i = 0; i < 4; i++) {

      int offset = 0;
      if (i > 0)
        offset = 2 * i;
      SDL_Rect area = {nametableArea.x + nametableArea.w * i + offset,
                       nametableArea.y, nametableArea.w, nametableArea.h};

      SDL_RenderCopy(gRenderer, EmuBus->ppu->getNameTable(i, p_idx), nullptr,
                     &area);
    }

    DisplayNESColorPalettes(gRenderer, EmuBus->ppu,
                            patternTableArea1.x + 2 * patternTableArea1.w + 200,
                            patternTableArea1.y, 16, 10);
#endif
    SDL_RenderPresent(gRenderer);
  }

  close_program();
  return 0;
}

void init() {

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK |
               SDL_INIT_GAMECONTROLLER) < 0) {
    fprintf(stderr, "Error Occoured: %s \n ", SDL_GetError());
    exit(1);
  }

  gWindow = SDL_CreateWindow("NedNes", SDL_WINDOWPOS_UNDEFINED,
                             SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH,
                             WINDOW_HEIGHT, SDL_WINDOW_SHOWN);

  if (!gWindow) {
    fprintf(stderr, "Couldn't Create Window: %s\n", SDL_GetError());
    exit(1);
  }

  gRenderer = SDL_CreateRenderer(
      gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

  if (!gRenderer) {
    fprintf(stderr, "Couldn't Create Renderer: %s\n", SDL_GetError());
    exit(1);
  }

  if (TTF_Init() < 0) {
    fprintf(stderr, "Couldn't Initalize TTF: %s \n", TTF_GetError());
    exit(1);
  }
  global_font = TTF_OpenFont("../asset/font/PixelEmulator-xq08.ttf", 17);
  if (!global_font) {
    fprintf(stderr, "Failed to Open Font: %s", TTF_GetError());
  } else {
    printf("Font Loaded Successfully\n");
  }
}

void close_program() {

  SDL_DestroyRenderer(gRenderer);
  gRenderer = nullptr;
  SDL_DestroyWindow(gWindow);
  gWindow = nullptr;
  SDL_Quit();
  printf("NedNes Closed Gracefully");
}
void HandleController(std::shared_ptr<NedNes::NedBus> bus) {
  uint8_t joypadState1 = 0;
  uint8_t joypadState2 = 0;
  if (mapped_joystick[0]) {

    joypadState1 = getControllerStateFromJoyStick(mapped_joystick[0]);
  } else {
    joypadState1 = getControllerStateFromKeyboard();
  }

  if (mapped_joystick[1]) {
    joypadState2 = getControllerStateFromJoyStick(mapped_joystick[1]);
  } else {
    joypadState2 = getControllerStateFromKeyboard();
  }
  bus->setState(0, joypadState1);
  bus->setState(1, joypadState2);
}
uint8_t getControllerStateFromKeyboard() {
  const Uint8 *state = SDL_GetKeyboardState(NULL);

  // Initialize the joypad state to zero
  uint8_t joypadState = 0;

  // Check for key presses and update joypad state
  for (const auto &pair : keymap) {
    if (state[SDL_GetScancodeFromKey(pair.first)]) {
      joypadState |= (1 << pair.second); // Set the corresponding bit
    }
  }
  return joypadState;
}
uint8_t getControllerStateFromJoyStick(SDL_GameController *ctrl) {

  uint8_t joypadState = 0;
  for (const auto &pair : joystickMap) {

    if (SDL_GameControllerGetButton(ctrl, pair.first)) {
      joypadState |= (1 << pair.second); // Set the corresponding bit
    }
  }
  return joypadState;
}
