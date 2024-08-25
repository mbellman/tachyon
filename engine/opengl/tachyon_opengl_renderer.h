#pragma once

#include <glew.h>
#include <SDL_opengl.h>

#include "engine/tachyon_types.h"
#include "engine/opengl/tachyon_opengl_framebuffer.h"
#include "engine/opengl/tachyon_opengl_geometry.h"
#include "engine/opengl/tachyon_opengl_shaders.h"

struct tOpenGLRenderer {
  SDL_GLContext gl_context;
  GLuint indirect_buffer;
  tOpenGLShaders shaders;
  // @todo will we need multiple mesh packs?
  tOpenGLMeshPack mesh_pack;

  struct tOpenGLRendererContext {
    uint32 w, h;

    tMat4f view_matrix;
    tMat4f projection_matrix;

    tMat4f inverse_view_matrix;
    tMat4f inverse_projection_matrix;
  } ctx;

  GLuint screen_quad_texture;
  tOpenGLScreenQuad screen_quad;

  OpenGLFrameBuffer g_buffer;

  // @todo dev mode only
  float last_shader_hot_reload_time = 0.f;
  uint64 last_render_time_in_microseconds = 0;
  bool show_debug_view = false;
};

void Tachyon_InitOpenGLRenderer(Tachyon* tachyon);
void Tachyon_RenderSceneInOpenGL(Tachyon* tachyon);
void Tachyon_OpenGL_RenderDeveloperOverlay(Tachyon* tachyon);
void Tachyon_DestroyOpenGLRenderer(Tachyon* tachyon);