#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>

#include "engine/tachyon_aliases.h"
#include "engine/tachyon_console.h"
#include "engine/tachyon_input.h"
#include "engine/tachyon_life_cycle.h"
#include "engine/tachyon_sound.h"
#include "engine/tachyon_timer.h"
#include "engine/tachyon_ui.h"
#include "engine/opengl/tachyon_opengl_renderer.h"

static void HandleEvents(Tachyon* tachyon) {
  SDL_Event event;

  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT:
        tachyon->is_running = false;
        break;
      case SDL_MOUSEBUTTONDOWN:
        if (!is_window_focused()) {
          // When re-focusing the window, stop here to avoid processing
          // the mousedown or any other events until the next frame.
          // We don't want the action to trigger a click within a game
          // if we're simply clicking back into the unfocused window.
          Tachyon_FocusWindow(tachyon);

          return;
        }

        break;
      case SDL_KEYDOWN:
        switch (event.key.keysym.sym) {
          case SDLK_ESCAPE:
            Tachyon_UnfocusWindow(tachyon);

            break;
        }
        break;
      case SDL_WINDOWEVENT:
        if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
          Tachyon_HandleWindowResize(tachyon);
        }
        break;

      case SDL_CONTROLLERDEVICEADDED: {
        tachyon->is_controller_connected = SDL_GameControllerOpen(0);

        break;
      }

      case SDL_CONTROLLERDEVICEREMOVED: {
        tachyon->left_stick = tVec2f(0.f);
        tachyon->right_stick = tVec2f(0.f);
        tachyon->left_trigger = 0.f;
        tachyon->right_trigger = 0.f;

        tachyon->is_controller_connected = false;

        break;
      }
    }

    Tachyon_HandleInputEvent(tachyon, event);
  }

  if (did_press_key(tKey::T)) {
    tachyon->show_developer_tools = !tachyon->show_developer_tools;
  }
}

static void RenderScene(Tachyon* tachyon) {
  if (tachyon->render_backend == TachyonRenderBackend::OPENGL) {
    Tachyon_OpenGL_RenderScene(tachyon);
  } else {
    SDL_Delay(16);
  }
}

static void DestroyRenderer(Tachyon* tachyon) {
  if (tachyon->render_backend == TachyonRenderBackend::OPENGL) {
    Tachyon_OpenGL_DestroyRenderer(tachyon);
  }

  delete tachyon->renderer;
}

Tachyon* Tachyon_Init() {
  SDL_Init(SDL_INIT_EVERYTHING);
  TTF_Init();
  IMG_Init(IMG_INIT_PNG);

  auto* tachyon = new Tachyon;

  Tachyon_InitSoundEngine();

  SDL_GameControllerAddMappingsFromFile("./controllers.txt");

  tachyon->is_controller_connected = SDL_GameControllerOpen(0);

  // @todo dev mode only
  tachyon->developer_overlay_font = TTF_OpenFont("./fonts/CascadiaMonoNF.ttf", 20);
  tachyon->alert_message_font = TTF_OpenFont("./fonts/OpenSans-Regular.ttf", 50);

  return tachyon;
}

void Tachyon_SpawnWindow(Tachyon* tachyon, const char* title, uint32 width, uint32 height) {
  tachyon->sdl_window = SDL_CreateWindow(
    title,
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    width, height,
    SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
  );

  tachyon->window_width = width;
  tachyon->window_height = height;
}

void Tachyon_UseRenderBackend(Tachyon* tachyon, TachyonRenderBackend backend) {
  if (tachyon->renderer != nullptr) {
    DestroyRenderer(tachyon);
  }

  tachyon->render_backend = backend;

  if (backend == TachyonRenderBackend::OPENGL) {
    Tachyon_OpenGL_InitRenderer(tachyon);
  }
}

void Tachyon_StartFrame(Tachyon* tachyon) {
  tachyon->frame_start_time_in_microseconds = Tachyon_GetMicroseconds();

  HandleEvents(tachyon);
}

void Tachyon_EndFrame(Tachyon* tachyon) {
  RenderScene(tachyon);

  Tachyon_ResetPerFrameInputState(tachyon);
  Tachyon_ClearUIDrawCommands(tachyon);
  Tachyon_ProcessConsoleMessages();

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

  // Ensure window unfocus events release the mouse held state
  tachyon->is_mouse_held_down = false;
}

void Tachyon_HandleWindowResize(Tachyon* tachyon) {
  int w, h;

  SDL_GL_GetDrawableSize(tachyon->sdl_window, &w, &h);

  tachyon->window_width = w;
  tachyon->window_height = h;

  if (tachyon->renderer != nullptr) {
    if (tachyon->render_backend == TachyonRenderBackend::OPENGL) {
      Tachyon_OpenGL_HandleWindowResize(tachyon);
    }
  }
}

void Tachyon_Exit(Tachyon* tachyon) {
  // @todo dev mode only
  TTF_CloseFont(tachyon->developer_overlay_font);
  
  Tachyon_ExitSoundEngine();

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