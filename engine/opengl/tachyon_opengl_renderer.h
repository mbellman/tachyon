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
    tMat4f view_projection_matrix;

    tMat4f inverse_view_matrix;
    tMat4f inverse_projection_matrix;

    tVec3f camera_position;
  } ctx;

  GLuint screen_quad_texture;
  tOpenGLScreenQuad screen_quad;

  OpenGLFrameBuffer g_buffer;
  OpenGLFrameBuffer accumulation_buffer_a;
  OpenGLFrameBuffer accumulation_buffer_b;

  uint32 current_frame = 0;

  // @todo dev mode only
  bool show_g_buffer_view = false;
  float last_shader_hot_reload_time = 0.f;
  uint64 last_render_time_in_microseconds = 0;
  uint32 total_triangles = 0;
  uint32 total_vertices = 0;
  uint32 total_meshes_drawn = 0;
  uint32 total_draw_calls = 0;
};

void Tachyon_OpenGL_InitRenderer(Tachyon* tachyon);
void Tachyon_OpenGL_ResizeRenderer(Tachyon* tachyon);
void Tachyon_OpenGL_RenderScene(Tachyon* tachyon);
void Tachyon_OpenGL_RenderDeveloperOverlay(Tachyon* tachyon);
void Tachyon_OpenGL_DestroyRenderer(Tachyon* tachyon);