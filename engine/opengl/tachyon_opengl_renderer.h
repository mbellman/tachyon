#pragma once

#include <SDL_opengl.h>

#include "engine/tachyon_types.h"

struct tOpenGLShader {
  GLuint program;
  GLuint vertex_shader;
  GLuint geometry_shader;
  GLuint fragment_shader;
};

struct tOpenGLShaders {
  tOpenGLShader main_geometry;
};

struct tOpenGLMeshPack {
  GLuint vao;
  GLuint buffers[3];
  GLuint ebo;
};

struct tOpenGLRenderer {
  SDL_GLContext context;
  GLuint indirect_buffer;
  tOpenGLShaders shaders;
  tOpenGLMeshPack mesh_pack;
};

void Tachyon_InitOpenGLRenderer(Tachyon* tachyon);
void Tachyon_RenderSceneInOpenGL(Tachyon* tachyon);
void Tachyon_DestroyOpenGLRenderer(Tachyon* tachyon);