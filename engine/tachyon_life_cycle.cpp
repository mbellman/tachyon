#include <SDL.h>

#include "engine/tachyon_life_cycle.h"
#include "engine/renderers/tachyon_opengl_renderer.h"

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

void Tachyon_UseRenderBackend(Tachyon* tachyon, TachyonRenderBackend backend) {
  if (tachyon->renderer != nullptr) {
    // @todo clean up existing renderer based on current backend

    delete tachyon->renderer;
  }

  tachyon->render_backend = backend;

  if (backend == TachyonRenderBackend::OPENGL) {
    Tachyon_InitOpenGLRenderer(tachyon);
  }
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

void Tachyon_RenderScene(Tachyon* tachyon) {
  if (tachyon->render_backend == TachyonRenderBackend::OPENGL) {
    Tachyon_RenderOpenGL(tachyon);
  } else {
    SDL_Delay(16);
  }
}

void Tachyon_Exit(Tachyon* tachyon) {
  if (tachyon->sdl_window != nullptr) {
    SDL_DestroyWindow(tachyon->sdl_window);
  }

  if (tachyon->renderer != nullptr) {
    delete tachyon->renderer;
  }

  delete tachyon;
}