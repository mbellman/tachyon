#include <SDL.h>

#include "engine/tachyon_init.h"

Tachyon* Tachyon_Init() {
  SDL_Init(SDL_INIT_EVERYTHING);

  return new Tachyon;
}

void Tachyon_CreateWindow(Tachyon* tachyon) {
  tachyon->sdl_window = SDL_CreateWindow(
    "Tachyon",
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    640, 480,
    SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
  );
}

bool Tachyon_IsRunning(Tachyon* tachyon) {
  return tachyon->running;
}

void Tachyon_HandleEvents(Tachyon* tachyon) {
  SDL_Event event;

  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT:
        tachyon->running = false;
        break;
    }
  }
}

void Tachyon_Exit(Tachyon* tachyon) {
  if (tachyon->sdl_window != nullptr) {
    SDL_DestroyWindow(tachyon->sdl_window);
  }
}