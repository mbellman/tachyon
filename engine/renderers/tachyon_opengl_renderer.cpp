#include <glew.h>
#include <SDL.h>
#include <SDL_opengl.h>

#include "engine/renderers/tachyon_opengl_renderer.h"

void Tachyon_InitOpenGLRenderer(Tachyon* tachyon) {
  auto* renderer = new TachyonOpenGLRenderer;

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

  renderer->context = SDL_GL_CreateContext(tachyon->sdl_window);

  glewExperimental = true;

  glewInit();

  SDL_GL_SetSwapInterval(0);

  // Apply default OpenGL settings
  glEnable(GL_PROGRAM_POINT_SIZE);
  glFrontFace(GL_CW);

  tachyon->renderer = renderer;
}

void Tachyon_RenderOpenGL(Tachyon* tachyon) {
  int w, h;

  SDL_GL_GetDrawableSize(tachyon->sdl_window, &w, &h);

  glViewport(0, 0, w, h);
  glClearColor(0, 0, 1.f, 1.f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  SDL_GL_SwapWindow(tachyon->sdl_window);
}

void Tachyon_DestroyOpenGLRenderer(Tachyon* tachyon) {
  auto* renderer = (TachyonOpenGLRenderer*)tachyon->renderer;

  SDL_GL_DeleteContext(renderer->context);
}