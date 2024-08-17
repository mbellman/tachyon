#pragma once

#include <SDL_opengl.h>

#include "engine/tachyon_types.h"

const enum tOpenGLMeshPackBuffer {
  VERTEX_BUFFER,
  COLOR_BUFFER,
  MATRIX_BUFFER
};

struct tOpenGLMeshPack {
  GLuint vao;
  GLuint buffers[3];
  GLuint ebo;
};

struct tOpenGLScreenQuad {
  GLuint vao;
  GLuint vbo;
};

tOpenGLMeshPack Tachyon_CreateOpenGLMeshPack(Tachyon* tachyon);
tOpenGLScreenQuad Tachyon_CreateOpenGLScreenQuad(Tachyon* tachyon);