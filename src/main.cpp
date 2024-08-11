#include "../include/NedNes.h"
#include <corecrt_wstdio.h>
#include <stdio.h>
#define _CRT_SECURE_NO_WARNINGS
#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_video.h>
#include <memory>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

SDL_Window *gWindow = nullptr;

SDL_Renderer *gRenderer = nullptr;

TTF_Font *global_font = nullptr;

void init();

void close_program();

int main(int argc, char **argv) {

  auto cart = std::make_shared<NedNes::NedCartrdige>("../tools/e.nes");

  // setting up nednes bus
  auto EmuBus = std::make_shared<NedNes::NedBus>();
  auto CPU = std::make_shared<NedNes::Ned6502>();
  auto PPU = std::make_shared<NedNes::Ned2C02>();
  PPU->connectBus(EmuBus);
  PPU->connectCart(cart);

  FILE *f = fopen("../rom/tests/nedlog.log", "wb");

  // Connecting everything to the bus

  EmuBus->connectCartridge(cart);
  EmuBus->connectPpu(PPU);
  EmuBus->connectCpu(CPU);
  CPU->connectBus(EmuBus);
  CPU->logFile = f;
  CPU->reset();
  printf("Program %s running with %d args\n", argv[0], argc);
  init();
  if (cart->imageValid()) {
    printf("Rom Loaded\n");
  }

  bool quit = false;

  SDL_Event e;
  while (!quit) {

    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) {
        quit = true;
      }
    }

    EmuBus->clock();
    SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0xff, 0x00);
    SDL_RenderClear(gRenderer);

    // stuff happens

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
  global_font = TTF_OpenFont("../asset/font/ARCADECLASSIC.TTF", 24);
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
