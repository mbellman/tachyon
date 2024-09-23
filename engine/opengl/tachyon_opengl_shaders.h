#pragma once

#include <string>
#include <vector>

#include <glew.h>
#include <SDL_opengl.h>

#define uniform_locations(...) struct {\
    GLint __VA_ARGS__;\
  }\

struct tOpenGLShaderAttachment {
  GLuint id;
  GLenum type;
  std::string source_path;
  std::vector<std::string> dependencies;
};

struct tOpenGLShader {
  GLuint program;

  std::vector<tOpenGLShaderAttachment> attachments;
};

struct tUniformLocations {
  uniform_locations(
    mat_view_projection,
    transform_origin
  ) main_geometry;

  uniform_locations(
    transform,
    in_normal_and_depth,
    in_color_and_material,
    inverse_projection_matrix,
    inverse_view_matrix,
    camera_position,
    // @temporary
    // @todo allow multiple directional lights
    directional_light_direction
  ) sky_and_directional_lighting;

  uniform_locations(
    transform,
    color,
    background
  ) surface;

  // @todo only in dev mode
  uniform_locations(
    transform,
    in_normal_and_depth,
    in_color_and_material,
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