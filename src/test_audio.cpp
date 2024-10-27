#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_log.h>
#include <SDL2/SDL_stdinc.h>
#include <cmath>
#include <iostream>

const int SAMPLE_RATE = 44100;
const int SAMPLE = 2048;
const int FREQUENCY = 440; // A4
const int AMP = 2000;
double t = 0;

double H = 40;
double duty_cycle = 0.75;
void AudioCallback(void *userdata, Uint8 *data, int len) {
  double timeStep = 1.0 / SAMPLE_RATE;
  int samples = len / sizeof(Sint16);
  Sint16 *buffer = (Sint16 *)(data);

  for (int i = 0; i < samples; i++) {
    double wave_a = 0;
    double wave_b = 0;

    for (int n = 1; n <= H; n++) {
      wave_a += (sin(2.0 * M_PI * FREQUENCY * t * n)) / n;
      /* wave_b+=(sin(2.0 * M_PI* ( FREQUENCY * t - duty_cycle)*n))/n; */
    }
    double wave = (wave_a - wave_b) * AMP;
    t += timeStep;
    buffer[i] = (Sint16)(wave);
  }
}
int main(int argc, char **argv) {

  if (SDL_Init(SDL_INIT_AUDIO) < 0) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to Init audio");
    exit(1);
  }

  SDL_AudioSpec want, have;
  SDL_AudioDeviceID device;

  SDL_zero(want);
  want.freq = SAMPLE_RATE;
  want.channels = 1; // mono channel
  want.samples = SAMPLE;
  want.format = AUDIO_S16; // 16 bit audio format
  want.callback = AudioCallback;

  device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);

  if (device == 0) {
    std::cerr << "Failed to open audio: " << SDL_GetError() << std::endl;
    return -1;
  }

  SDL_PauseAudioDevice(device, 0);
  std::cout << "press\n";
  std::cin.get();

  SDL_CloseAudioDevice(device);
  SDL_Quit();
  return 0;
}
