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
    view_projection_matrix,
    transform_origin,
    has_texture,
    albedo_texture,
    is_grass,
    foliage_mover_position,
    foliage_mover_velocity,
    scene_time
  ) main_geometry;

  uniform_locations(
    light_matrix,
    transform_origin
  ) shadow_map;

  uniform_locations(
    offset_and_scale,
    rotation,
    in_normal_and_depth,
    in_color_and_material,
    in_temporal_data,
    previous_color_and_depth,
    in_shadow_map_cascade_1,
    in_shadow_map_cascade_2,
    in_shadow_map_cascade_3,
    in_shadow_map_cascade_4,
    light_matrix_cascade_1,
    light_matrix_cascade_2,
    light_matrix_cascade_3,
    light_matrix_cascade_4,
    projection_matrix,
    view_matrix,
    previous_view_matrix,
    inverse_projection_matrix,
    inverse_view_matrix,
    camera_position,
    scene_time,
    running_time,
    // @todo allow multiple directional lights
    primary_light_direction,
    primary_light_color,
    player_position,
    fog_color,
    fog_visibility,
    use_high_visibility_mode,
    accumulation_blur_factor
  ) global_lighting;

  uniform_locations(
    in_normal_and_depth,
    in_color_and_material,
    projection_matrix,
    view_matrix,
    inverse_projection_matrix,
    inverse_view_matrix,
    camera_position,
    accumulation_blur_factor
  ) point_lights;

  uniform_locations(
    view_projection_matrix,
    transform_origin
  ) wireframe_mesh;

  uniform_locations(
    view_projection_matrix,
    transform_origin,
    camera_position,
    primary_light_direction,
    scene_time
  ) volumetric_mesh;

  uniform_locations(
    view_projection_matrix,
    transform_origin,
    camera_position,
    scene_time
  ) fire_mesh;

  uniform_locations(
    view_projection_matrix,
    transform_origin,
    scene_time
  ) ion_thruster_mesh;

  uniform_locations(
    view_projection_matrix,
    transform_origin,
    camera_position,
    primary_light_direction,
    previous_color_and_depth,
    scene_time
  ) water_mesh;

  uniform_locations(
    offset_and_scale,
    rotation,
    in_color_and_depth,
    inverse_projection_matrix,
    inverse_view_matrix,
    camera_position,
    primary_light_direction,

    // Fx: Cosmodrone
    scan_time,

    // Fx: Alchemist's Astrolabe
    player_position,
    astro_time_warp,
    astro_time_warp_start_radius,
    astro_time_warp_end_radius,
    vignette_intensity
  ) post;

  uniform_locations(
    offset_and_scale,
    rotation,
    color,
    background
  ) surface;

  // @todo only in dev mode
  uniform_locations(
    offset_and_scale,
    rotation,
    in_normal_and_depth,
    in_color_and_material,
    inverse_projection_matrix,
    inverse_view_matrix
  ) debug_view;
};

struct tOpenGLShaders {
  tOpenGLShader main_geometry;
  tOpenGLShader shadow_map;
  tOpenGLShader global_lighting;
  tOpenGLShader point_lights;
  tOpenGLShader volumetric_mesh;
  tOpenGLShader fire_mesh;
  tOpenGLShader ion_thruster_mesh;
  tOpenGLShader wireframe_mesh;
  tOpenGLShader water_mesh;
  tOpenGLShader post;

  tOpenGLShader surface;

  // @todo dev mode only
  tOpenGLShader debug_view;

  tUniformLocations locations;
};

void Tachyon_OpenGL_InitShaders(tOpenGLShaders& shaders);
void Tachyon_OpenGL_HotReloadShaders(tOpenGLShaders& shaders);
void Tachyon_OpenGL_DestroyShaders(tOpenGLShaders& shaders);