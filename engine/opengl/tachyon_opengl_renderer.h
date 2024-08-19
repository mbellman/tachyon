#pragma once

#include <SDL_opengl.h>

#include "engine/tachyon_types.h"
#include "engine/opengl/tachyon_opengl_framebuffer.h"
#include "engine/opengl/tachyon_opengl_geometry.h"

// @todo dev mode only
enum DebugView {
  DEFAULT,
  NORMALS,
  DEPTH,
  MATERIAL
};

struct tOpenGLShader {
  GLuint program;
  GLuint vertex_shader;
  GLuint geometry_shader;
  GLuint fragment_shader;
};

struct tOpenGLShaders {
  tOpenGLShader main_geometry;
  tOpenGLShader sky_and_directional_lighting;

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
  } ctx;

  GLuint screen_quad_texture;
  tOpenGLScreenQuad screen_quad;

  OpenGLFrameBuffer g_buffer;

  // @todo dev mode only
  uint64 last_render_time_in_microseconds = 0;
  uint8 debug_view = DebugView::DEFAULT;
};

void Tachyon_InitOpenGLRenderer(Tachyon* tachyon);
void Tachyon_RenderSceneInOpenGL(Tachyon* tachyon);
void Tachyon_OpenGL_RenderDeveloperOverlay(Tachyon* tachyon);
void Tachyon_DestroyOpenGLRenderer(Tachyon* tachyon);