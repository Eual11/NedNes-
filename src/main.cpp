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
#include <corecrt_wstdio.h>
#include <memory>
#include <stdio.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
bool bFreeRun = false;
SDL_Window *gWindow = nullptr;
int p_idx = 0;
SDL_Renderer *gRenderer = nullptr;

TTF_Font *global_font = nullptr;

// Screen and pattern table area
//
SDL_Rect scrArea = {0, 0, 256, 240};
SDL_Rect patternTableArea1 = {0, 400, 128, 128};
SDL_Rect patternTableArea2 = {256, 400, 128, 128};

void init();

void close_program();

int main(int argc, char **argv) {

  init();
  auto cart = std::make_shared<NedNes::NedCartrdige>(
      "../rom/games/Super Mario Bros (E).nes");

  // setting up nednes bus
  auto EmuBus = std::make_shared<NedNes::NedBus>();
  auto CPU = std::make_shared<NedNes::Ned6502>();
  auto PPU = std::make_shared<NedNes::Ned2C02>(gRenderer);
  PPU->connectBus(EmuBus);
  PPU->connectCart(cart);

  EmuBus->connectCartridge(cart);
  EmuBus->connectPpu(PPU);
  EmuBus->connectCpu(CPU);
  CPU->connectBus(EmuBus);
  /* CPU->logFile = f; */
  CPU->reset();
  printf("Program %s running with %d args\n", argv[0], argc);
  if (cart->imageValid()) {
    printf("Rom Loaded\n");
  }

  bool quit = false;
  std::map<uint16_t, std::string> disMap;

  SDL_Event e;
  while (!quit) {

    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) {
        quit = true;
      }

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
          // step one clock

          EmuBus->clock();

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
    SDL_RenderCopy(gRenderer, EmuBus->ppu->getScreenTexture(), nullptr,
                   &scrArea);
    SDL_RenderCopy(gRenderer, EmuBus->ppu->getPatternTable(0, p_idx), nullptr,
                   &patternTableArea1);
    SDL_RenderCopy(gRenderer, EmuBus->ppu->getPatternTable(1, p_idx), nullptr,
                   &patternTableArea2);

    DisplayNESColorPalettes(gRenderer, EmuBus->ppu, 400, 400, 16, 10);
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

void close_program()

{

  SDL_DestroyRenderer(gRenderer);
  gRenderer = nullptr;
  SDL_DestroyWindow(gWindow);
  gWindow = nullptr;
  SDL_Quit();
  printf("NedNes Closed Gracefully");
}
