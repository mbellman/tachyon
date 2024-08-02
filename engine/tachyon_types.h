#pragma once

struct SDL_Window;

enum TachyonRenderBackend {
  OPENGL
};

struct Tachyon {
  SDL_Window* sdl_window = nullptr;
  TachyonRenderBackend render_backend = TachyonRenderBackend::OPENGL;
  void* renderer = nullptr;
  bool running = true;
};