#ifndef _RENDER_UTILS_HPP
#define _RENDER_UTILS_HPP
#include "../include/Ned2CO2.h"
#include "Ned6502.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>
#include <cstdint>
#include <sstream>
#include <string>

SDL_Rect DrawText(SDL_Renderer *renderer, TTF_Font *font, std::string str,
                  int x, int y, SDL_Color col);
SDL_Rect DrawCPUReg(std::shared_ptr<NedNes::Ned6502> cpu,
                    SDL_Renderer *renderer, TTF_Font *font, int x, int y,
                    SDL_Color col);
void DisplayNESColorPalettes(SDL_Renderer *renderer,
                             std::shared_ptr<NedNes::Ned2C02> ppu, int startX,
                             int startY, int swatchSize, int spacing);
void DrawRect(SDL_Renderer *renderer, int x, int y, int w, int h, Uint32 color);
std::string toHex(int number);
#endif
