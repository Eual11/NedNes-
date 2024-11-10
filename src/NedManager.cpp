#include "../include/NedManager.h"
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_log.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_video.h>
using namespace NedNes;

NedManager::NedManager() {
  keymap = {

      {SDLK_a, NedNes::BUTTON_A},       {SDLK_d, NedNes::BUTTON_B},

      {SDLK_q, NedNes::BUTTON_SELECT},  {SDLK_e, NedNes::BUTTON_START},
      {SDLK_UP, NedNes::BUTTON_UP},     {SDLK_DOWN, NedNes::BUTTON_DOWN},
      {SDLK_LEFT, NedNes::BUTTON_LEFT}, {SDLK_RIGHT, NedNes::BUTTON_RIGHT},
  };
  joystickMap = {

      {SDL_CONTROLLER_BUTTON_A, NedNes::BUTTON_A},
      {SDL_CONTROLLER_BUTTON_B, NedNes::BUTTON_B},
      {SDL_CONTROLLER_BUTTON_BACK, NedNes::BUTTON_SELECT},
      {SDL_CONTROLLER_BUTTON_START, NedNes::BUTTON_START},

      {SDL_CONTROLLER_BUTTON_DPAD_UP, NedNes::BUTTON_UP},
      {SDL_CONTROLLER_BUTTON_DPAD_DOWN, NedNes::BUTTON_DOWN},
      {SDL_CONTROLLER_BUTTON_DPAD_LEFT, NedNes::BUTTON_LEFT},
      {SDL_CONTROLLER_BUTTON_DPAD_RIGHT, NedNes::BUTTON_RIGHT},
  };

  mapped_joystick[0] = nullptr;
  mapped_joystick[1] = nullptr;
}
bool NedManager::Init() {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK |
               SDL_INIT_GAMECONTROLLER) < 0) {
    fprintf(stderr, "Error Occoured: %s \n ", SDL_GetError());
    return false;
  }

  gWindow = SDL_CreateWindow("NedNes", SDL_WINDOWPOS_UNDEFINED,
                             SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH,
                             WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
  if (!gWindow) {
    fprintf(stderr, "Couldn't Create Window: %s\n", SDL_GetError());
    exit(1);
  }
  if ((IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) & (IMG_INIT_JPG | IMG_INIT_PNG)) !=
      (IMG_INIT_PNG | IMG_INIT_JPG)) {
    fprintf(stderr, "Couldn't Initalize SDL_Image  %s\n", IMG_GetError());
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
  for (int i = 0; i < SDL_NumJoysticks(); i++) {
    if (SDL_IsGameController((i))) {
      SDL_GameController *ctrler = SDL_GameControllerOpen(i);
      SDL_Log("controller %d added\n", SDL_GameControllerNameForIndex(i));
      if (ctrler) {
        joysticks.insert(ctrler);
      }
    }
  }

  background =
      IMG_LoadTexture(gRenderer, "../asset/backgrounds/background_1.png");

  header = IMG_LoadTexture(gRenderer, "../asset/headers/header1.png");
  gameicon = IMG_LoadTexture(gRenderer, "../asset/game_icons/megaman.png");
  SDL_QueryTexture(header, nullptr, nullptr, &headerRect.w, &headerRect.h);
  SDL_QueryTexture(gameicon, nullptr, nullptr, &gameiconRect.w,
                   &gameiconRect.h);
  NED = NedNesEmulator(gRenderer);
  Initalized = true;

  return true;
}
void NedManager::Close() {

  SDL_DestroyRenderer(gRenderer);
  SDL_CloseAudioDevice(device);
  gRenderer = nullptr;
  SDL_DestroyWindow(gWindow);
  gWindow = nullptr;
  TTF_CloseFont(global_font);
  IMG_Quit();
  SDL_Quit();
  printf("NedNes Closed Gracefully");
}
void NedManager::HandleController(std::shared_ptr<NedNes::NedBus> bus) {
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
uint8_t NedManager::getControllerStateFromKeyboard() {
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
uint8_t NedManager::getControllerStateFromJoyStick(SDL_GameController *ctrl) {

  uint8_t joypadState = 0;
  for (const auto &pair : joystickMap) {

    if (SDL_GameControllerGetButton(ctrl, pair.first)) {
      joypadState |= (1 << pair.second); // Set the corresponding bit
    }
  }

  // checking for analog sticks
  Sint16 leftX = SDL_GameControllerGetAxis(ctrl, SDL_CONTROLLER_AXIS_LEFTX);
  // Get the value of the left stick's vertical axis (Y)
  Sint16 leftY = SDL_GameControllerGetAxis(ctrl, SDL_CONTROLLER_AXIS_LEFTY);
  // Get the value of the right stick's horizontal axis (X)

  if (leftX > 0) {

    joypadState |= (1 << NedNes::BUTTON_RIGHT);
  } else if (leftX < 0) {

    joypadState |= (1 << NedNes::BUTTON_LEFT);
  }

  // BUG: for some reason SDL reports moving the axis upward as a negative
  // value? why?
  if (leftY > 0) {

    joypadState |= (1 << NedNes::BUTTON_DOWN);
  } else if (leftY < 0) {

    joypadState |= (1 << NedNes::BUTTON_UP);
  }

  return joypadState;
}
void NedManager::Run() {
  if (!Initalized) {
    SDL_LogError(SDL_LogCategory::SDL_LOG_CATEGORY_ASSERT,
                 "Please Call Init before running\n Exiting...\n");
    exit(1);
  }
  while (!Quit) {
    while (SDL_PollEvent(&cur_event)) {
      HandleEvents(cur_event);
    }
    SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0xff, 0x00);
    SDL_RenderClear(gRenderer);

    if (gameRunning) {
      HandleController(NED.getBus());

      if (!gamePaused) {
        NED.stepFrame();
      }
      /* RenderCPUState(); */
      /* RenderPPUState(); */
      SDL_RenderCopy(gRenderer, NED.getNewFrame(), nullptr, &scrArea);
    } else {
      // render UI
      RenderUI();
    }
    SDL_RenderPresent(gRenderer);
  }
  Close();
}
void NedManager::HandleEvents(SDL_Event &event) {
  switch (event.type) {
  case SDL_QUIT: {
    Quit = true;
    break;
  }
  case SDL_CONTROLLERDEVICEADDED: {
    AddController(event);
    break;
  }
  case SDL_CONTROLLERDEVICEREMOVED: {
    RemoveController(event);
    break;
  }
  case SDL_KEYDOWN: {
    switch (event.key.keysym.sym) {
    case SDLK_ESCAPE: {
      // quit if game is running
      NED.unload();
      gameRunning = false;
      break;
    }
    case SDLK_p: {
      if (!gameRunning) {
        if (NED.loadRom("../rom/games/Super Mario Bros (E).nes")) {
          gamePaused = false;
          gameRunning = true;
        }
      }
      break;
    }
    }
    break;
  }
  }
}
void NedManager::AddController(SDL_Event &event) {
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
    // HACK: this is a hack for testing the joysticks
    mapped_joystick[0] = *joysticks.begin();
    SDL_Log("%s is now Player 1\n",
            SDL_GameControllerNameForIndex(event.cdevice.which));
  }
  auto iter = joysticks.begin();
  if (joysticks.size() > 1) {
    for (int i = 0; i < 2; i++) {
      mapped_joystick[i] = *iter;
      SDL_Log("%s is now Player %d \n", SDL_GameControllerName(*iter), i + 1);
      iter++;
    }
  }
}
void NedManager::RemoveController(SDL_Event &event) {
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
void NedManager::RenderCPUState() {
  SDL_Color col = {0xff, 0xff, 0xff, 0xff};

  SDL_Rect R = DrawCPUReg(NED.getCPU(), gRenderer, global_font,
                          WINDOW_WIDTH - 350, 10, col);
  R.h += 40;
  // Drawing disassembled instructions
  disMap = NED.getDissmap();
  col = {0x04, 0x09c, 0xf4, 0xff};
  for (auto &d : disMap) {
    std::string instr = toHex(d.first) + " " + d.second;
    SDL_Rect r = DrawText(gRenderer, global_font, instr, WINDOW_WIDTH - 350,
                          R.h + 10, col);
    R.h += r.h;
    col = {0xff, 0xff, 0xff, 0x00};
  }
}
void NedManager::RenderPPUState() {

  SDL_RenderCopy(gRenderer, NED.getPPU()->getPatternTable(0, 0), nullptr,
                 &patternTableArea1);
  SDL_RenderCopy(gRenderer, NED.getPPU()->getPatternTable(1, 0), nullptr,
                 &patternTableArea2);

  for (int i = 0; i < 4; i++) {

    int offset = 0;
    if (i > 0)
      offset = 2 * i;
    SDL_Rect area = {nametableArea.x + nametableArea.w * i + offset,
                     nametableArea.y, nametableArea.w, nametableArea.h};

    SDL_RenderCopy(gRenderer, NED.getPPU()->getNameTable(i, 0), nullptr, &area);
  }

  DisplayNESColorPalettes(gRenderer, NED.getPPU(),
                          patternTableArea1.x + 2 * patternTableArea1.w + 200,
                          patternTableArea1.y, 16, 10);
}

void NedManager::RenderUI() {

  SDL_RenderCopy(gRenderer, background, nullptr, nullptr);
  SDL_RenderCopy(gRenderer, header, nullptr, &headerRect);
  SDL_RenderCopy(gRenderer, gameicon, nullptr, &gameiconRect);
}
