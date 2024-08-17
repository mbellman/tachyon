#pragma once

#include <SDL_opengl.h>

#include "engine/tachyon_types.h"
#include "engine/opengl/tachyon_opengl_geometry.h"

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

struct tOpenGLRenderer {
  SDL_GLContext gl_context;
  GLuint indirect_buffer;
  tOpenGLShaders shaders;
  // @todo will we need multiple mesh packs?
  tOpenGLMeshPack mesh_pack;

  struct tOpenGLRendererContext {
    uint32 w, h;

    uint64 last_frame_time_in_microseconds = 0;
  } ctx;

  GLuint screen_quad_texture;
  tOpenGLScreenQuad screen_quad;
};

void Tachyon_InitOpenGLRenderer(Tachyon* tachyon);
void Tachyon_RenderSceneInOpenGL(Tachyon* tachyon);
void Tachyon_OpenGL_RenderDeveloperOverlay(Tachyon* tachyon);
void Tachyon_DestroyOpenGLRenderer(Tachyon* tachyon);