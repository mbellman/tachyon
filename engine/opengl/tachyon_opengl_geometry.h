#pragma once

#include <SDL_opengl.h>

#include "engine/tachyon_types.h"

enum tOpenGLMeshPackBuffer {
  VERTEX_BUFFER,
  SURFACE_BUFFER,
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

struct tOpenGLLightDisc {
  GLuint vao;
  GLuint vertex_buffer;
  GLuint light_buffer;
};

struct tOpenGLPointLightDiscInstance {
  tVec2f offset;
  tVec2f scale;
  tPointLight light;
};

tOpenGLMeshPack Tachyon_CreateOpenGLMeshPack(Tachyon* tachyon);
tOpenGLScreenQuad Tachyon_CreateOpenGLScreenQuad(Tachyon* tachyon);
tOpenGLLightDisc Tachyon_CreateOpenGLPointLightDisc(Tachyon* tachyon);