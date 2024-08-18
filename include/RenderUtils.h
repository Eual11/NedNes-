#ifndef _RENDER_UTILS_HPP
#define _RENDER_UTILS_HPP
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
std::string toHex(int number);
#endif
