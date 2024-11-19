#include "../include/NedManager.h"
#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_log.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_video.h>
#include <cassert>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
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

  gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
  if (!gRenderer) {
    fprintf(stderr, "Couldn't Create Renderer: %s\n", SDL_GetError());
    exit(1);
  }
  if (TTF_Init() < 0) {
    fprintf(stderr, "Couldn't Initalize TTF: %s \n", TTF_GetError());
    exit(1);
  }

  LoadConfigFile();
  SetupAudio();
  global_font = TTF_OpenFont("../asset/font/Pixeboy.ttf", 39);
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
  /**/
  /* images["background"] = std::make_shared<Image>( */
  /*     gRenderer, "../asset/backgrounds/background_1.png "); */
  /**/
  loadImage("background", "../asset/backgrounds/background_1.png", WINDOW_WIDTH,
            WINDOW_HEIGHT);

  SDL_Surface *icon = SDL_LoadBMP("../asset/game_icons/window_icon.bmp");
  SDL_SetWindowIcon(this->gWindow, icon);

  SDL_FreeSurface(icon);

  loadImage("play_icon", "../asset/btn_icons/play_icon.png");
  loadImage("next_icon", "../asset/btn_icons/next_icon.png");
  loadImage("prev_icon", "../asset/btn_icons/prev_icon.png");
  loadImage("exit_icon", "../asset/btn_icons/exit_icon.png");
  loadImage("cheveron", "../asset/btn_icons/cheveron.png");
  loadImage("music_icon", "../asset/btn_icons/music Icon.png", 32, 32);
  images["music_icon"]->setPos(700, 105);

  auto playBtn = createButton("Play", 300, 200, images["play_icon"]);
  playBtn->setOnClick([this]() {
    int idx =
        GameMenu->getSelectedIDX() + current_page * program_count_per_page;
    if (idx >= 0 && idx < programs_list.size()) {
      RunProgram(programs_list[idx].second);
    }
  });

  auto nextBtn = createButton("Next", 0, 0, images["next_icon"]);
  nextBtn->setOnClick([this] { UpdateGameMenu(current_page + 1); });

  auto prevBtn = createButton("Prev", 0, 0, images["prev_icon"]);
  prevBtn->setOnClick([this] { UpdateGameMenu(current_page - 1); });
  auto exitBtn = createButton("Exit", 0, 0, images["exit_icon"]);
  exitBtn->setOnClick([this]() { Quit = true; });

  // Arrange buttons horizontally
  arrangeButtonsHorizontally(25, 440,
                             20); // Start from (300, 400) with 20px gap

  background =
      IMG_LoadTexture(gRenderer, "../asset/backgrounds/background_1.png");

  header = IMG_LoadTexture(gRenderer, "../asset/headers/header1.png");
  gameicon = IMG_LoadTexture(gRenderer, "../asset/game_icons/megaman.png");
  SDL_QueryTexture(header, nullptr, nullptr, &headerRect.w, &headerRect.h);
  SDL_QueryTexture(gameicon, nullptr, nullptr, &gameiconRect.w,
                   &gameiconRect.h);

  SDL_Color col;
  col.r = 0xFF;
  col.g = 0x00;
  col.b = 0x00;
  PageLabel = std::make_unique<Label>("Page 1", 25, 485, PageTextColor,
                                      gRenderer, global_font);
  GameMenu = std::make_unique<SelectionMenu>(60, 160, PageTextColor,
                                             PageHighlightColor, gRenderer,
                                             global_font, images["cheveron"]);
  UpdateGameMenu(current_page);
  NED = NedNesEmulator(gRenderer, device);
  Initalized = true;

  return true;
}
void NedManager::Close() {

  NED.getAPU()->writeWAV("../test.wav");
  SDL_DestroyRenderer(gRenderer);
  SDL_CloseAudioDevice(device);
  SDL_CloseAudio();
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

    uint32_t frame_start = SDL_GetTicks();
    while (SDL_PollEvent(&cur_event)) {
      HandleEvents(cur_event);

      if (!gameRunning) {
        for (auto &btn : buttons) {
          btn->HandleEvents(cur_event);
        }
        GameMenu->HandleEvents(cur_event);
      }
    }
    SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0xff, 0x00);
    SDL_RenderClear(gRenderer);

    if (gameRunning) {
      HandleController(NED.getBus());

      if (!gamePaused) {
        NED.stepFrame();
      }
      SDL_RenderCopy(gRenderer, NED.getNewFrame(), nullptr, &scrArea);
    } else {
      // render UI
      RenderUI();
    }
    SDL_RenderPresent(gRenderer);

    Uint32 frame_time =
        SDL_GetTicks() - frame_start; // Time taken to render the frame
    if (frame_time < TARGET_FRAMETIME) {
      SDL_Delay(TARGET_FRAMETIME - frame_time); // Wait for the remaining time
    }
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
      HaltEmulator();
      break;
    }
    case SDLK_p: {

      UpdateGameMenu(current_page - 1);
      break;
    }
    case SDLK_n: {
      UpdateGameMenu(current_page + 1);
      break;
    }
    case SDLK_m: {

      SDL_PauseAudioDevice(device, muted ? 1 : 0);
      muted = !muted;
      break;
    }
    case SDLK_RETURN: {
      //  SDL_Log();
      int idx =
          GameMenu->getSelectedIDX() + current_page * program_count_per_page;
      if (idx >= 0 && idx < programs_list.size()) {
        RunProgram(programs_list[idx].second);
      }
      break;
    }
    case SDLK_c: {
      GameMenu->Clear();
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

  /* SDL_RenderCopy(gRenderer, background, nullptr, nullptr); */

  if (images["background"]) {
    images["background"]->Render(gRenderer);
  }
  if (images["music_icon"] && !muted) {
    (images["music_icon"]->Render(gRenderer));
  }
  SDL_RenderCopy(gRenderer, header, nullptr, &headerRect);
  SDL_RenderCopy(gRenderer, gameicon, nullptr, &gameiconRect);
  PageLabel->Render(gRenderer);
  for (auto &btn : buttons) {
    btn->Render(gRenderer);
  }
  GameMenu->Render();
}

void NedManager::RunProgram(std::string path) {
  if (!gameRunning) {
    if (NED.loadRom(path)) {
      gamePaused = false;
      gameRunning = true;
    }
  }
}
void NedManager::HaltEmulator() {
  NED.unload();
  gameRunning = false;
}
void NedManager::UpdateGameMenu(int page) {

  if (page * program_count_per_page < programs_list.size() && page >= 0) {

    int start_idx = program_count_per_page * page;
    GameMenu->Clear();
    int i = 0;
    while ((start_idx + i) < programs_list.size() &&
           i < program_count_per_page) {
      GameMenu->addLabel(programs_list[start_idx + i].first);
      i += 1;
    }
    current_page = page;
    if (PageLabel) {
      PageLabel->setText("Page " + std::to_string(current_page + 1),
                         PageTextColor, gRenderer, global_font);
    }
    SDL_Log("Now on page %d\n", current_page);
  }
}
Label::Label(std::string text_, int x, int y, SDL_Color col,
             SDL_Renderer *renderer, TTF_Font *text_font) {

  this->text = text_;
  Pos.x = x;
  Pos.y = y;
  color = col;
  this->font = text_font;

  SDL_Surface *text_surface =
      TTF_RenderText_Solid(font, this->text.c_str(), {0xFF, 0xFF, 0XFF, 0XFF});
  if (text_surface) {
    texture = SDL_CreateTextureFromSurface(renderer, text_surface);

    if (texture) {
      Pos.x = x;
      Pos.y = y;
      Pos.w = text_surface->w;
      Pos.h = text_surface->h;
    }
    SDL_FreeSurface(text_surface);
  }
}
void Label::Render(SDL_Renderer *renderer) const {
  if (texture) {
    SDL_SetTextureColorMod(texture, color.r, color.g, color.b);
    SDL_RenderCopy(renderer, texture, nullptr, &Pos);
  }
}
void Label::setText(std::string new_text, SDL_Color col, SDL_Renderer *renderer,
                    TTF_Font *new_font) {
  text = new_text;
  if (new_font) {
    this->font = new_font;
  }
  if (texture)
    SDL_DestroyTexture(texture);
  color = col;
  SDL_Surface *text_surface =
      TTF_RenderText_Solid(font, new_text.c_str(), {0xFF, 0xFF, 0xFF, 0xFF});

  if (text_surface) {

    texture = SDL_CreateTextureFromSurface(renderer, text_surface);

    if (texture) {
      Pos.w = text_surface->w;
      Pos.h = text_surface->h;
    }
    SDL_FreeSurface(text_surface);
  }
}
Label::~Label() {
  if (texture)
    SDL_DestroyTexture(texture);
  font = nullptr;
  texture = nullptr;
}
Image::Image(SDL_Renderer *gRenderer, std::string path, int x, int y) {
  texture = IMG_LoadTexture(gRenderer, path.c_str());

  if (!texture) {
    SDL_Log("Error: %s", SDL_GetError());
  } else {
    rect.x = x;
    rect.y = y;
    SDL_QueryTexture(texture, nullptr, nullptr, &rect.w, &rect.h);
  }
}

Image::~Image() {
  if (texture) {
    SDL_DestroyTexture(texture);
  }
  texture = nullptr;
}
void Image::Render(SDL_Renderer *renderer) const {
  if (texture)
    SDL_RenderCopy(renderer, texture, nullptr, &rect);
}

Button::Button(std::string str_txt, int x, int y, SDL_Color tcol,
               SDL_Color hcol, SDL_Renderer *renderer, TTF_Font *font,
               std::shared_ptr<Image> icon_) {

  text = std::make_unique<Label>(str_txt, x, y, tcol, renderer, font);

  text_color = tcol;
  highlight_color = hcol;
  icon = icon_;
  if (icon) {
    icon->setPos(x, y);
    rect = {x, y, text->getRect().w + icon->getRect().w,
            std::max(icon->getRect().h, text->getRect().h)};
    text->setPos(x + icon_->getRect().w + gap, y + rect.h / 4);

  } else {
    rect = {x, y, text->getRect().w, text->getRect().h};
    text->setPos(x, y);
  }
}
void Button::HandleEvents(SDL_Event &event) {

  int Mx = event.motion.x;
  int My = event.motion.y;
  int x = rect.x;
  int y = rect.y;
  int w = rect.w;
  int h = rect.h;

  if ((Mx <= x + w) && (Mx >= x) && (My >= y) && (My <= y + h)) {
    hovered = true;
    if (onHover) {
      onHover();
    }
  } else
    hovered = false;

  if (event.type == SDL_MOUSEBUTTONDOWN && (Mx <= x + w) && (Mx >= x) &&
      (My >= y) && (My <= y + h)) {
    if (event.button.button == SDL_BUTTON_LEFT) {
      if (onClick) {
        onClick();
      }
    }
  }
}

void Button::Render(SDL_Renderer *renderer) const {
  if (icon) {
    icon->Render(renderer);
  }
  if (text) {
    text->setColor(text_color);
    if (hovered) {
      text->setColor(highlight_color);
    }
    text->Render(renderer);
  }
}
SelectionMenu::SelectionMenu(int x, int y, SDL_Color tcol, SDL_Color hcol,
                             SDL_Renderer *_renderer, TTF_Font *font,
                             std::shared_ptr<Image> ico) {
  rect.x = x;
  rect.y = y;
  text_color = tcol;
  highlight_color = hcol;
  renderer = _renderer;
  this->font = font;
  select_icon = ico;
}

void SelectionMenu::addLabel(std::string text) {
  text = std::to_string(selection_labels.size() + 1) + ". " + text;

  int x = rect.x;
  int y = rect.y + rect.h;
  auto new_label =
      std::make_unique<Label>(text, x, y, text_color, renderer, font);

  rect.h += (new_label->getRect().h + gap);
  selection_labels.push_back(std::move(new_label));
}
void SelectionMenu::addLabels(std::vector<std::string> texts) {

  for (auto &lb : texts) {
    addLabel(lb);
  }
}
void SelectionMenu::HandleEvents(SDL_Event &event) {
  if (event.type == SDL_KEYDOWN) {

    switch (event.key.keysym.sym) {
    case SDLK_DOWN: {
      if (selection_labels.empty()) {
        idx = 0;
      } else
        idx = (idx + 1) % selection_labels.size();
      break;
      ;
    }
    case SDLK_UP: {
      if (selection_labels.empty())
        idx = 0;
      else {
        idx -= 1;
        if (idx < 0) {
          idx = selection_labels.size() - 1;
        }
      }

      break;
    }
    }
  }
}
void SelectionMenu::Render() const {
  int i = 0;
  for (auto &label : selection_labels) {

    if (i == idx) {
      label->setColor(highlight_color);
      if (select_icon) {
        select_icon->setPos(label->getRect().x - select_icon->getRect().w - gap,
                            label->getRect().y - 2);
      }
    } else {
      label->setColor(text_color);
    }
    label->Render(renderer);
    i += 1;
  }
  if (select_icon && !selection_labels.empty())
    select_icon->Render(renderer);
}

void SelectionMenu::Clear() {
  selection_labels.clear();
  rect = {rect.x, rect.y, 0, 0};
  idx = 0;
}
std::shared_ptr<Button> NedManager::createButton(const std::string &text, int x,
                                                 int y,
                                                 std::shared_ptr<Image> icon) {
  auto button =
      std::make_shared<Button>(text, x, y, PageTextColor, PageHighlightColor,
                               gRenderer, global_font, icon);
  buttons.push_back(button);
  return button;
}
void NedManager::loadImage(const std::string &key, const std::string &filePath,
                           int width, int height) {
  auto image = std::make_shared<Image>(gRenderer, filePath);
  if (width > 0 && height > 0) {
    image->setSize(width, height);
  }
  images[key] = std::move(image);
}
void NedManager::arrangeButtonsHorizontally(int startX, int startY,
                                            int spacing) {
  int xPos = startX;
  for (auto &button : buttons) {
    button->setPosition(xPos, startY); // Set button position
    xPos +=
        button->getRect().w + spacing; // Move to the next position with spacing
  }
}

void NedManager::SetupAudio() {

  SDL_AudioSpec want, have;

  SDL_zero(want);

  want.freq = SAMPLE_RATE;
  want.format = AUDIO_FORMAT;
  want.channels = CHANNELS_COUNT;
  want.samples = SAMPLE;

  device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);

  if (device == 0) {
    std::cerr << "Failed to Open Audio: " << SDL_GetError() << std::endl;
  } else {
    std::cout << "Audio Device Created\n";
  }

  // unpausing Audio device
  // NOTE: this will be changed after testing

  SDL_PauseAudioDevice(device, muted);
}

void NedManager::Callback(void *userdata, Uint8 *stream, int len) {

  // silencing the buffer
  memset(stream, 0, len);

  static int phase = 0;
  NedManager *manager = static_cast<NedManager *>(userdata);
  assert(manager != nullptr);

  Sint16 *buffer = (Sint16 *)(stream);

  len = len / (sizeof(Sint16));
  manager->getEmu()->fillAudioBuffer(buffer, len);
#if 0
  int amp = 10000;
  uint16_t freq = 440; // A4
  for (int i = 0; i < len; i++) {

    float time = (float)(phase) / manager->SAMPLE_RATE;
    double sine_sample = 0.5 * sin(2 * 3.14159 * 440 * time);

    phase += 1;
    if (phase >= manager->SAMPLE_RATE)
      phase -= manager->SAMPLE_RATE;

    // Scale the result to fit into the 16-bit audio format
    buffer[i] = static_cast<int16_t>(sine_sample * INT16_MAX);
  }
#endif
  auto &ab = manager->audioBuffer;
  ab.insert(ab.end(), buffer, buffer + len);
}
void NedManager::LoadConfigFile() {

  // open config file

  std::fstream config("../config/config.ini", std::ios::in);

  if (!config.is_open()) {
    std::cerr << "Failed to Open Config File\n Try Creating config directory "
                 "and a file with name config.ini\n";
    return;
    ;
  }
  // store it to parsed config

  std::stringstream buffer;

  std::string cur_section = "";

  // a disgusting hack
  std::vector<std::string> sections = {"[games]", "[settings]"};

  std::string line;

  while (std::getline(config, line)) {

    if (std::find(sections.begin(), sections.end(), line) != sections.end()) {

      cur_section = line;
      continue;
    }

    if (cur_section != "")
      parsed_config[cur_section] += line + "\n";
  }

  // Process games and store our lovely games

  ProcessGamesSection();

  // process emulator settings

  ProcessSettings();
}

void NedManager::ProcessGamesSection() {

  std::istringstream prg_list(parsed_config["[games]"]);

  std::string line;

  while (std::getline(prg_list, line)) {

    // removing any Quotations

    size_t eq = line.find_first_of("=");

    std::string title = line.substr(0, eq);
    std::string path = line.substr(eq + 1);
    programs_list.push_back({title, path});
  }
}
void NedManager::ProcessSettings() {
  std::istringstream prg_list(parsed_config["[settings]"]);

  std::string line;

  while (std::getline(prg_list, line)) {

    // removing any Quotations

    size_t eq = line.find_first_of("=");

    std::string key = line.substr(0, eq);
    std::string val = line.substr(eq + 1);

    if (key == "mute") {

      if (val == "true") {
        muted = true;
      } else if (val == "false")
        muted = false;
    }
    if (key == "fps") {
      try {
        int fps = std::stoi(val);
        TARGET_FRAMETIME = 1000 / fps;
      } catch (const std::invalid_argument &) {
      } catch (const std::out_of_range &) {
      }
    }
  }
}
