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
    in_temporal_data,
    projection_matrix,
    view_matrix,
    inverse_projection_matrix,
    inverse_view_matrix,
    camera_position,
    scene_time,
    running_time,
    // @temporary
    // @todo allow multiple directional lights
    directional_light_direction
  ) global_lighting;

  uniform_locations(
    transform,
    in_color_and_depth
  ) post;

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
  tOpenGLShader global_lighting;
  tOpenGLShader post;

  tOpenGLShader surface;

  // @todo dev mode only
  tOpenGLShader debug_view;

  tUniformLocations locations;
};

void Tachyon_OpenGL_InitShaders(tOpenGLShaders& shaders);
void Tachyon_OpenGL_HotReloadShaders(tOpenGLShaders& shaders);
void Tachyon_OpenGL_DestroyShaders(tOpenGLShaders& shaders);