#include "../include/RenderUtils.h"
#include <SDL2/SDL_hints.h>
SDL_Rect DrawText(SDL_Renderer *renderer, TTF_Font *font, std::string str,
                  int x, int y, SDL_Color col) {

  SDL_Rect rect;
  SDL_Surface *text = TTF_RenderText_Solid(font, str.c_str(), col);
  if (text) {
    SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text);

    if (text_texture) {
      rect.x = x;
      rect.y = y;
      rect.w = text->w;
      rect.h = text->h;

      SDL_RenderCopy(renderer, text_texture, nullptr, &rect);
      SDL_DestroyTexture(text_texture);
    }
    SDL_FreeSurface(text);
  }
  return rect;
}
std::string toHex(int number) {
  std::stringstream ss;
  ss << "0x" << std::uppercase << std::hex << number;
  return ss.str();
}

SDL_Rect DrawCPUReg(std::shared_ptr<NedNes::Ned6502> cpu,
                    SDL_Renderer *renderer, TTF_Font *font, int x, int y,
                    SDL_Color col) {
  // visialize the stauts register

  // visialize program counter

  SDL_Color green;
  green.g = 255;
  green.r = 0x00;
  green.b = 0x00;
  green.a = 0x00;
  SDL_Color red;
  red.r = 255;
  red.a = 0x00;
  red.g = 0x00;
  red.b = 0x00;
  SDL_Rect R;
  int Xpos, Ypos;
  Xpos = x;
  Ypos = y;
  int line_spacing = 2;
  int spacing = 2;

  // CPU Regesters
  uint8_t A = cpu->A;
  uint8_t X = cpu->X;
  uint8_t Y = cpu->Y;
  uint16_t PC = cpu->PC;
  uint8_t STKP = cpu->STKP;

  // Visualizing Status Register;

  uint8_t status = cpu->status;

  R = DrawText(renderer, font, "Status: ", Xpos, Ypos, col);
  Xpos += R.w + spacing;
  // Netive FLag
  R = DrawText(renderer, font, "N ", Xpos, Ypos,
               cpu->getFlag(NedNes::Ned6502::NedCPUFlags::N) ? green : red);
  Xpos += R.w + spacing;
  R = DrawText(renderer, font, "V ", Xpos, Ypos,
               cpu->getFlag(NedNes::Ned6502::NedCPUFlags::V) ? green : red);
  Xpos += R.w + spacing;
  R = DrawText(renderer, font, "_ ", Xpos, Ypos, col);
  Xpos += R.w + spacing;
  R = DrawText(renderer, font, "B ", Xpos, Ypos,
               cpu->getFlag(NedNes::Ned6502::NedCPUFlags::B) ? green : red);
  Xpos += R.w + spacing;
  R = DrawText(renderer, font, "D ", Xpos, Ypos,
               cpu->getFlag(NedNes::Ned6502::NedCPUFlags::D) ? green : red);
  Xpos += R.w + spacing;
  R = DrawText(renderer, font, "I ", Xpos, Ypos,
               cpu->getFlag(NedNes::Ned6502::NedCPUFlags::I) ? green : red);
  Xpos += R.w + spacing;
  R = DrawText(renderer, font, "Z ", Xpos, Ypos,
               cpu->getFlag(NedNes::Ned6502::NedCPUFlags::Z) ? green : red);
  Xpos += R.w + spacing;
  R = DrawText(renderer, font, "C ", Xpos, Ypos,
               cpu->getFlag(NedNes::Ned6502::NedCPUFlags::C) ? green : red);
  Xpos += R.w + spacing;

  Ypos += R.h + line_spacing;

  Xpos = x;
  // Drawing Program Counter and other registers
  R = DrawText(renderer, font, "PC: " + toHex(PC), Xpos, Ypos, col);
  Ypos += R.h + line_spacing;

  R = DrawText(renderer, font,
               "A: " + toHex(A) + "   [" + std::to_string(A) + "]", Xpos, Ypos,
               col);
  Ypos += R.h + line_spacing;
  //"[" + std::to_string(A) + "]"
  R = DrawText(renderer, font,
               "X: " + toHex(X) + "   [" + std::to_string(X) + "]", Xpos, Ypos,
               col);
  Ypos += R.h + line_spacing;
  R = DrawText(renderer, font,
               "Y: " + toHex(Y) + "   [" + std::to_string(Y) + "]", Xpos, Ypos,
               col);
  Ypos += R.h + line_spacing;
  R = DrawText(renderer, font,
               "STKP: " + toHex(STKP) + "   [" + std::to_string(STKP) + "]",
               Xpos, Ypos, col);

  R.h = Ypos + line_spacing;
  return R;
}
