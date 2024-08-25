#pragma once

#include <glew.h>
#include <SDL_opengl.h>

#define uniform_locations(...) struct {\
    GLint __VA_ARGS__;\
  }\

struct tOpenGLShader {
  GLuint program;
  GLuint vertex_shader;
  GLuint geometry_shader;
  GLuint fragment_shader;
};

struct tUniformLocations {
  uniform_locations(
    mat_view_projection
  ) main_geometry;

  uniform_locations(
    transform,
    in_color_and_depth,
    in_normal_and_material,
    inverse_projection_matrix,
    inverse_view_matrix
  ) sky_and_directional_lighting;

  uniform_locations(
    transform,
    color,
    background
  ) surface;

  // @todo only in dev mode
  uniform_locations(
    transform,
    in_color_and_depth,
    in_normal_and_material,
    inverse_projection_matrix,
    inverse_view_matrix
  ) debug_view;
};

struct tOpenGLShaders {
  tOpenGLShader main_geometry;
  tOpenGLShader sky_and_directional_lighting;

  tOpenGLShader surface;

  // @todo dev mode only
  tOpenGLShader debug_view;

  tUniformLocations locations;
};

void Tachyon_OpenGL_InitShaders(tOpenGLShaders& shaders);
void Tachyon_OpenGL_HotReloadShaders(tOpenGLShaders& shaders);
void Tachyon_OpenGL_DestroyShaders(tOpenGLShaders& shaders);