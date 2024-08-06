#include <SDL.h>

#include "engine/tachyon_aliases.h"
#include "engine/tachyon_life_cycle.h"
#include "engine/opengl/tachyon_opengl_renderer.h"

internal void DestroyRenderer(Tachyon* tachyon) {
  if (tachyon->render_backend == TachyonRenderBackend::OPENGL) {
    Tachyon_DestroyOpenGLRenderer(tachyon);
  }

  delete tachyon->renderer;
}

Tachyon* Tachyon_Init() {
  SDL_Init(SDL_INIT_EVERYTHING);

  return new Tachyon;
}

void Tachyon_SpawnWindow(Tachyon* tachyon, const char* title, uint32 width, uint32 height) {
  tachyon->sdl_window = SDL_CreateWindow(
    title,
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    width, height,
    SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
  );
}

void Tachyon_UseRenderBackend(Tachyon* tachyon, TachyonRenderBackend backend) {
  if (tachyon->renderer != nullptr) {
    DestroyRenderer(tachyon);
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
      case SDL_MOUSEBUTTONDOWN:
        Tachyon_FocusWindow();
        break;
      case SDL_KEYDOWN:
        switch (event.key.keysym.sym) {
          case SDLK_ESCAPE:
            Tachyon_UnfocusWindow();
            break;
        }
        break;
    }
  }
}

void Tachyon_RenderScene(Tachyon* tachyon) {
  if (tachyon->render_backend == TachyonRenderBackend::OPENGL) {
    Tachyon_RenderSceneInOpenGL(tachyon);
  } else {
    SDL_Delay(16);
  }
}

void Tachyon_FocusWindow() {
  SDL_SetRelativeMouseMode(SDL_TRUE);
}

void Tachyon_UnfocusWindow() {
  SDL_SetRelativeMouseMode(SDL_FALSE);
}

void Tachyon_Exit(Tachyon* tachyon) {
  if (tachyon->sdl_window != nullptr) {
    SDL_DestroyWindow(tachyon->sdl_window);
  }

  if (tachyon->renderer != nullptr) {
    DestroyRenderer(tachyon);
  }

  delete tachyon;
}