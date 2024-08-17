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

  tOpenGLShader surface;
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

struct tOpenGLRenderer {
  SDL_GLContext context;
  GLuint indirect_buffer;
  tOpenGLShaders shaders;
  // @todo will we need multiple mesh packs?
  tOpenGLMeshPack mesh_pack;

  GLuint screen_quad_texture;
  tOpenGLScreenQuad screen_quad;
};

void Tachyon_InitOpenGLRenderer(Tachyon* tachyon);
void Tachyon_RenderSceneInOpenGL(Tachyon* tachyon);
void Tachyon_OpenGL_RenderDeveloperOverlay(Tachyon* tachyon);
void Tachyon_DestroyOpenGLRenderer(Tachyon* tachyon);