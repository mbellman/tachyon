#pragma once

#include <SDL_opengl.h>

#include "engine/tachyon_types.h"

struct TachyonOpenGLShader {
  GLuint program;
  GLuint vertex_shader;
  GLuint geometry_shader;
  GLuint fragment_shader;
};

struct TachyonOpenGLShaders {
  TachyonOpenGLShader main_geometry;
};

struct TachyonOpenGLRenderer {
  SDL_GLContext context;

  TachyonOpenGLShaders shaders;
};

void Tachyon_InitOpenGLRenderer(Tachyon* tachyon);
void Tachyon_RenderSceneInOpenGL(Tachyon* tachyon);
void Tachyon_DestroyOpenGLRenderer(Tachyon* tachyon);