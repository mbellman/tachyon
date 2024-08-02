#pragma once

struct SDL_Window;

struct Tachyon {
  SDL_Window* sdl_window = nullptr;
  bool running = true;
};