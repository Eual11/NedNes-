#include <SDL2/SDL_keycode.h>
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
#include <stdio.h>

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

void init();

void close_program();

int main(int argc, char **argv) {

  // TODO: better controllers
  init();
  auto cart = std::make_shared<NedNes::NedCartrdige>(
      "../rom/games/Arkista's Ring (U) [!].nes");

  auto joypad1 = std::make_shared<NedNes::NedJoypad>();
  // setting up nednes bus
  auto EmuBus = std::make_shared<NedNes::NedBus>();
  auto CPU = std::make_shared<NedNes::Ned6502>();
  auto PPU = std::make_shared<NedNes::Ned2C02>(gRenderer);
  PPU->connectBus(EmuBus);
  PPU->connectCart(cart);

  EmuBus->connectCartridge(cart);
  EmuBus->connectPpu(PPU);
  EmuBus->connectCpu(CPU);
  EmuBus->connectJoypad(0, joypad1);
  CPU->connectBus(EmuBus);
  /* CPU->logFile = f; */

  EmuBus->reset();
  printf("Program %s running with %d args\n", argv[0], argc);
  if (cart->imageValid()) {
    printf("Rom Loaded\n");
  }

  bool quit = false;
  std::map<uint16_t, std::string> disMap;

  std::map<SDL_KeyCode, NedNes::JOYPAD_BUTTONS> keymap = {

      {SDLK_a, NedNes::BUTTON_A},       {SDLK_d, NedNes::BUTTON_B},

      {SDLK_q, NedNes::BUTTON_SELECT},  {SDLK_e, NedNes::BUTTON_START},
      {SDLK_UP, NedNes::BUTTON_UP},     {SDLK_DOWN, NedNes::BUTTON_DOWN},
      {SDLK_LEFT, NedNes::BUTTON_LEFT}, {SDLK_RIGHT, NedNes::BUTTON_RIGHT},
  };
  SDL_Event e;
  while (!quit) {
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) {
        quit = true;
      }

      // Get the current state of the keyboard
      const Uint8 *state = SDL_GetKeyboardState(NULL);

      // Initialize the joypad state to zero
      uint8_t joypadState = 0;

      // Check for key presses and update joypad state
      for (const auto &pair : keymap) {
        if (state[SDL_GetScancodeFromKey(pair.first)]) {
          joypadState |= (1 << pair.second); // Set the corresponding bit
        }
      }
      EmuBus->setState(0, joypadState);

      // Update the joypad state

      if (e.type == SDL_KEYDOWN) {
        switch (e.key.keysym.sym) {
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

    SDL_RenderCopy(
        gRenderer, EmuBus->ppu->getScreenTexture(), nullptr,
        &scrArea); /* SDL_RenderCopy(gRenderer, EmuBus->ppu->getPatternTable(0,
                    * p_idx), nullptr, */
    /*                &patternTableArea1); */
    /* SDL_RenderCopy(gRenderer, EmuBus->ppu->getPatternTable(1, p_idx),
     * nullptr, */
    /*                &patternTableArea2); */

    /* for (int i = 0; i < 4; i++) { */
    /**/
    /*   int offset = 0; */
    /*   if (i > 0) */
    /*     offset = 2 * i; */
    /*   SDL_Rect area = {nametableArea.x + nametableArea.w * i + offset, */
    /*                    nametableArea.y, nametableArea.w, nametableArea.h};
     */
    /*   SDL_RenderCopy(gRenderer, EmuBus->ppu->getNameTable(i, p_idx),
     * nullptr, */
    /*                  &area); */
    /* } */
    /**/
    /* DisplayNESColorPalettes(gRenderer, EmuBus->ppu, */
    /*                         patternTableArea1.x + 2 * patternTableArea1.w +
     * 200, */
    /*                         patternTableArea1.y, 16, 10); */
    SDL_RenderPresent(gRenderer);
  }

  close_program();
  return 0;
}

void init() {

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
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

  gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);

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
