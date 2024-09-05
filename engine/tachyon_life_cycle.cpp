#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>

#include "engine/tachyon_aliases.h"
#include "engine/tachyon_input.h"
#include "engine/tachyon_life_cycle.h"
#include "engine/tachyon_timer.h"
#include "engine/opengl/tachyon_opengl_renderer.h"

internal void HandleEvents(Tachyon* tachyon) {
  SDL_Event event;

  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT:
        tachyon->is_running = false;
        break;
      case SDL_MOUSEBUTTONDOWN:
        Tachyon_FocusWindow(tachyon);
        break;
      case SDL_KEYDOWN:
        switch (event.key.keysym.sym) {
          case SDLK_ESCAPE:
            Tachyon_UnfocusWindow(tachyon);
            break;
        }
        break;
    }

    Tachyon_HandleInputEvent(tachyon, event);
  }
}

internal void RenderScene(Tachyon* tachyon) {
  if (tachyon->render_backend == TachyonRenderBackend::OPENGL) {
    Tachyon_RenderSceneInOpenGL(tachyon);
  } else {
    SDL_Delay(16);
  }
}

internal void DestroyRenderer(Tachyon* tachyon) {
  if (tachyon->render_backend == TachyonRenderBackend::OPENGL) {
    Tachyon_DestroyOpenGLRenderer(tachyon);
  }

  delete tachyon->renderer;
}

Tachyon* Tachyon_Init() {
  SDL_Init(SDL_INIT_EVERYTHING);
  TTF_Init();
  IMG_Init(IMG_INIT_PNG);

  auto* tachyon = new Tachyon;

  // @todo dev mode only
  tachyon->developer_overlay_font = TTF_OpenFont("./fonts/OpenSans-Regular.ttf", 20);

  return tachyon;
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
  return tachyon->is_running;
}

void Tachyon_StartFrame(Tachyon* tachyon) {
  tachyon->frame_start_time_in_microseconds = Tachyon_GetMicroseconds();

  HandleEvents(tachyon);
}

void Tachyon_EndFrame(Tachyon* tachyon) {
  RenderScene(tachyon);
  Tachyon_ResetPerFrameState(tachyon);

  tachyon->last_frame_time_in_microseconds = Tachyon_GetMicroseconds() - tachyon->frame_start_time_in_microseconds;
  tachyon->running_time += (float)tachyon->last_frame_time_in_microseconds / 1000000.f;
  tachyon->dev_labels.clear();
}

void Tachyon_FocusWindow(Tachyon* tachyon) {
  SDL_SetRelativeMouseMode(SDL_TRUE);

  tachyon->is_window_focused = true;
}

void Tachyon_UnfocusWindow(Tachyon* tachyon) {
  SDL_SetRelativeMouseMode(SDL_FALSE);

  tachyon->is_window_focused = false;
}

void Tachyon_Exit(Tachyon* tachyon) {
  // @todo dev mode only
  TTF_CloseFont(tachyon->developer_overlay_font);

  if (tachyon->renderer != nullptr) {
    DestroyRenderer(tachyon);
  }

  if (tachyon->sdl_window != nullptr) {
    SDL_DestroyWindow(tachyon->sdl_window);
  }

  IMG_Quit();
  TTF_Quit();

  delete tachyon;
}