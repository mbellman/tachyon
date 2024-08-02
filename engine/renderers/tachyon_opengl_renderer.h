#pragma once

#include <SDL_opengl.h>

#include "engine/tachyon_types.h"

struct TachyonOpenGLRenderer {
  SDL_GLContext context;
};

void Tachyon_InitOpenGLRenderer(Tachyon* tachyon);
void Tachyon_RenderOpenGL(Tachyon* tachyon);