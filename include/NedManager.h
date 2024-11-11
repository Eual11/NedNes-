#ifndef __NED_MANGER_H
#define __NED_MANGER_H

#include "NedBus.h"
#include "NedNes.h"
#include "RenderUtils.h"
#include <SDL2/SDL_gamecontroller.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <cstdint>
#include <memory>
#include <set>

namespace NedNes {
class Label {

private:
  std::string text;
  TTF_Font *font = nullptr;
  SDL_Rect Pos;
  SDL_Color color;
  SDL_Texture *texture = nullptr;

public:
  Label() = default;
  Label(std::string, int x, int y, SDL_Color col, SDL_Renderer *,
        TTF_Font *font = nullptr);
  ~Label();
  SDL_Rect getRect() const { return Pos; };
  void Render(SDL_Renderer *) const;
  std::string getText() const { return text; };
  SDL_Color getColor() const { return color; }
  void setText(std::string text, SDL_Color col, SDL_Renderer *,
               TTF_Font *font = nullptr);
  void setPos(int x, int y) {
    Pos.x = x;
    Pos.y = y;
  }
  void setColor(SDL_Color col) { color = col; }
};
class Image {
private:
  SDL_Texture *texture = nullptr;
  SDL_Rect rect = {0, 0, 0, 0};

public:
  Image() = default;
  Image(SDL_Renderer *, std::string path, int x = 0, int y = 0);
  ~Image();
  SDL_Rect getRect() const { return rect; }
  void setPos(int x, int y) {
    rect.x = x;
    rect.y = y;
  }
  void setSize(int w, int h) {
    rect.w = w;
    rect.h = h;
  }
  void Render(SDL_Renderer *renderer) const;
};

class Button {
public:
  using Callback = std::function<void()>;
  void Render(SDL_Renderer *) const;

  Button(std::string, int x, int y, SDL_Color tcol, SDL_Color hcol,
         SDL_Renderer *, TTF_Font *font = nullptr,
         std::shared_ptr<Image> icon = nullptr);
  void setOnClick(Callback cbk) { onClick = cbk; };
  void setOnHover(Callback cbk) { onHover = cbk; };
  void HandleEvents(SDL_Event &);
  void setPosition(int x, int y) {
    if (icon) {
      rect = {x, y, text->getRect().w + icon->getRect().w,
              std::max(icon->getRect().h, text->getRect().h)};
      text->setPos(rect.x + icon->getRect().w, y);
    } else {
      rect = {x, y, text->getRect().w, text->getRect().h};
      text->setPos(x, y);
    }
  }

private:
  SDL_Rect rect = {0, 0, 0, 0};
  SDL_Color text_color;
  SDL_Color highlight_color;
  bool hovered = false;
  bool pressed = false;
  int gap = 8;
  std::unique_ptr<Label> text;
  std::shared_ptr<Image> icon;
  Callback onClick = nullptr;
  Callback onHover = nullptr;
};

class SelectionMenu {
private:
  std::vector<std::unique_ptr<Label>> selection_labels;
  std::shared_ptr<Image> select_icon;
  SDL_Rect rect = {0, 0, 0, 0};
  SDL_Color text_color;
  SDL_Color highlight_color;
  int idx = 0;
  TTF_Font *font = nullptr;
  int gap = 16;
  SDL_Renderer *renderer = nullptr;

public:
  SelectionMenu() = default;
  SelectionMenu(int x, int y, SDL_Color tcol, SDL_Color hcol,
                SDL_Renderer *renderer, TTF_Font *font,
                std::shared_ptr<Image> ico = nullptr);
  SDL_Rect getRect() const { return rect; }
  int getSelectedIDX() const {
    if (selection_labels.empty())
      return -1;
    return idx;
  }
  void setColor(SDL_Color col) { text_color = col; }
  void setFont(TTF_Font *font) { this->font = font; }
  void setHighlightColor(SDL_Color col) { highlight_color = col; }
  void setPos(int x, int y) {
    rect.x = x;
    rect.y = y;
  }
  void addLabel(std::string text);
  void addLabels(std::vector<std::string>);
  void Render() const;
  void selectIdx(unsigned int new_idx) {
    idx = new_idx % selection_labels.size();
  }
  void HandleEvents(SDL_Event &);
  void Clear();
};

class NedManager {
  // singleton of the emulaotr
  // TODO: exception handling
private:
  NedNesEmulator NED;

  bool gameRunning = false;
  bool gamePaused = false;
  bool Quit = false;
  bool Initalized = false;
  SDL_Window *gWindow = nullptr;
  SDL_Renderer *gRenderer = nullptr;

  std::map<std::string, std::shared_ptr<Image>> images;

  std::vector<std::shared_ptr<Button>> buttons;

  // window parameters
  int WINDOW_WIDTH = 800;
  int WINDOW_HEIGHT = 600;
  // a global font and willa also act as a fallback font when we
  // implement the pages
  TTF_Font *global_font = nullptr;
  SDL_AudioDeviceID device = 0;

  // current event pulled from SDL_Event queue

  SDL_Event cur_event;
  // Rendering rect areas for emulator, including debuging

  SDL_Rect scrArea = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
  SDL_Rect patternTableArea1 = {0, (WINDOW_HEIGHT - (128 + 30)), 128, 128};
  SDL_Rect patternTableArea2 = {256, (WINDOW_HEIGHT - (128 + 30)), 128, 128};
  SDL_Rect nametableArea = {0, 300, 256, 240};

  // controller and keyboard mapping
  std::map<SDL_KeyCode, NedNes::JOYPAD_BUTTONS> keymap;
  std::map<SDL_GameControllerButton, NedNes::JOYPAD_BUTTONS> joystickMap;

  // all conencted controllers
  std::set<SDL_GameController *> joysticks;

  // mapped joystics we use for controller purpose, a nullptr means keyboard
  // controllered
  SDL_GameController *mapped_joystick[2];

  // control handlers
  void HandleController(std::shared_ptr<NedNes::NedBus> bus);
  uint8_t getControllerStateFromKeyboard();
  uint8_t getControllerStateFromJoyStick(SDL_GameController *);

  void AddController(SDL_Event &);
  void RemoveController(SDL_Event &);
  // debug
  void RenderCPUState();
  void RenderPPUState();
  std::map<uint16_t, std::string> disMap;

  // UI STUFF
  SDL_Texture *background = nullptr;
  SDL_Texture *header = nullptr;
  SDL_Texture *gameicon = nullptr;

  SDL_Rect headerRect = {250, 90, 0, 0};
  SDL_Rect gameiconRect{450, 200, 0};
  std::unique_ptr<Label> PageLabel;
  std::unique_ptr<SelectionMenu> GameMenu;
  SDL_Color PageTextColor = {0x48, 0x24, 0x13};
  SDL_Color PageHighlightColor = {0x0D, 0x79, 0xDE};

  void RenderUI();

public:
  NedManager();
  // intilaizes everything
  bool Init();
  void Close();
  void Run();
  void HandleEvents(SDL_Event &);
  void RunProgram(std::string);
  void HaltEmulator();
};
}; // namespace NedNes

#endif
