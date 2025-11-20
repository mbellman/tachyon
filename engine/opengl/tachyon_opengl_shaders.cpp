#include <filesystem>
#include <string>
#include <vector>

#include "engine/tachyon_aliases.h"
#include "engine/tachyon_console.h"
#include "engine/tachyon_file_helpers.h"
#include "engine/opengl/tachyon_opengl_shaders.h"

struct FileRecord {
  std::string path;
  std::filesystem::file_time_type last_write_time;
};

static std::vector<tOpenGLShader*> shader_list;
static std::vector<FileRecord> source_file_records;
static std::vector<std::string> changed_source_paths;

static void SaveSourceFileRecord(const char* path) {
  for (auto& record : source_file_records) {
    if (record.path.compare(path) == 0) {
      return;
    }
  }

  FileRecord record;

  record.path = path;
  record.last_write_time = std::filesystem::last_write_time(std::filesystem::current_path() / path);

  source_file_records.push_back(record);
}

static tOpenGLShaderAttachment AttachShader(tOpenGLShader& shader, GLenum type, const char* path) {
  auto id = glCreateShader(type);
  auto shader_file_contents = Tachyon_GetFileContents(path);
  auto* shader_file_contents_pointer = shader_file_contents.c_str();

  // @todo handle #includes/other directives etc.

  SaveSourceFileRecord(path);

  glShaderSource(id, 1, &shader_file_contents_pointer, 0);
  glCompileShader(id);

  // @todo dev mode only
  {
    GLint status;

    glGetShaderiv(id, GL_COMPILE_STATUS, &status);

    if (status != GL_TRUE) {
      char error[512];

      glGetShaderInfoLog(id, 512, 0, error);

      printf("Failed to compile shader: %s\n", path);
      printf("\u001b[31m %s\n\u001b[37m", error);

      add_console_message("Failed to compile shader: " + std::string(path), tVec3f(1.f, 0.8f, 0.2f));
      // @todo split error by newlines and add each as its own message
      add_console_message(error, tVec3f(1.f, 0, 0));
    }
  }

  glAttachShader(shader.program, id);

  return {
    .id = id,
    .type = type,
    .source_path = path,
    .dependencies = {
      // @todo pass all included files
      path
    }
  };
}

static void StoreShaderUniforms(tOpenGLShaders& shaders) {
  #define store_shader_uniform(shader_name, uniform_name) \
    shaders.locations.shader_name.uniform_name = glGetUniformLocation(shaders.shader_name.program, #uniform_name);

  store_shader_uniform(main_geometry, view_projection_matrix);
  store_shader_uniform(main_geometry, transform_origin);
  store_shader_uniform(main_geometry, has_texture);
  store_shader_uniform(main_geometry, albedo_texture);
  store_shader_uniform(main_geometry, is_grass);
  store_shader_uniform(main_geometry, foliage_mover_position);
  store_shader_uniform(main_geometry, foliage_mover_velocity);
  store_shader_uniform(main_geometry, scene_time);


  store_shader_uniform(shadow_map, light_matrix);
  store_shader_uniform(shadow_map, transform_origin);

  store_shader_uniform(global_lighting, offset_and_scale);
  store_shader_uniform(global_lighting, rotation);
  store_shader_uniform(global_lighting, in_normal_and_depth);
  store_shader_uniform(global_lighting, in_color_and_material);
  store_shader_uniform(global_lighting, in_temporal_data);
  store_shader_uniform(global_lighting, previous_color_and_depth);
  store_shader_uniform(global_lighting, in_shadow_map_cascade_1);
  store_shader_uniform(global_lighting, in_shadow_map_cascade_2);
  store_shader_uniform(global_lighting, in_shadow_map_cascade_3);
  store_shader_uniform(global_lighting, in_shadow_map_cascade_4);
  store_shader_uniform(global_lighting, light_matrix_cascade_1);
  store_shader_uniform(global_lighting, light_matrix_cascade_2);
  store_shader_uniform(global_lighting, light_matrix_cascade_3);
  store_shader_uniform(global_lighting, light_matrix_cascade_4);
  store_shader_uniform(global_lighting, projection_matrix);
  store_shader_uniform(global_lighting, view_matrix);
  store_shader_uniform(global_lighting, previous_view_matrix);
  store_shader_uniform(global_lighting, inverse_projection_matrix);
  store_shader_uniform(global_lighting, inverse_view_matrix);
  store_shader_uniform(global_lighting, camera_position);
  store_shader_uniform(global_lighting, scene_time);
  store_shader_uniform(global_lighting, running_time);
  // @todo allow multiple directional lights
  store_shader_uniform(global_lighting, primary_light_direction);
  store_shader_uniform(global_lighting, primary_light_color);
  store_shader_uniform(global_lighting, fog_color);
  store_shader_uniform(global_lighting, fog_visibility);
  store_shader_uniform(global_lighting, accumulation_blur_factor);
  store_shader_uniform(global_lighting, use_high_visibility_mode);

  store_shader_uniform(point_lights, in_normal_and_depth);
  store_shader_uniform(point_lights, in_color_and_material);
  store_shader_uniform(point_lights, projection_matrix);
  store_shader_uniform(point_lights, view_matrix);
  store_shader_uniform(point_lights, inverse_projection_matrix);
  store_shader_uniform(point_lights, inverse_view_matrix);
  store_shader_uniform(point_lights, camera_position);
  store_shader_uniform(point_lights, accumulation_blur_factor);

  store_shader_uniform(wireframe_mesh, view_projection_matrix);
  store_shader_uniform(wireframe_mesh, transform_origin);

  store_shader_uniform(volumetric_mesh, view_projection_matrix);
  store_shader_uniform(volumetric_mesh, transform_origin);
  store_shader_uniform(volumetric_mesh, camera_position);
  store_shader_uniform(volumetric_mesh, primary_light_direction);
  store_shader_uniform(volumetric_mesh, scene_time);

  store_shader_uniform(fire_mesh, view_projection_matrix);
  store_shader_uniform(fire_mesh, transform_origin);
  store_shader_uniform(fire_mesh, camera_position);
  store_shader_uniform(fire_mesh, scene_time);

  store_shader_uniform(ion_thruster_mesh, view_projection_matrix);
  store_shader_uniform(ion_thruster_mesh, transform_origin);
  store_shader_uniform(ion_thruster_mesh, scene_time);

  store_shader_uniform(water_mesh, view_projection_matrix);
  store_shader_uniform(water_mesh, transform_origin);
  store_shader_uniform(water_mesh, camera_position);
  store_shader_uniform(water_mesh, primary_light_direction);
  store_shader_uniform(water_mesh, previous_color_and_depth);
  store_shader_uniform(water_mesh, scene_time);

  store_shader_uniform(post, offset_and_scale);
  store_shader_uniform(post, rotation);
  store_shader_uniform(post, in_color_and_depth);
  store_shader_uniform(post, inverse_projection_matrix);
  store_shader_uniform(post, inverse_view_matrix);
  store_shader_uniform(post, camera_position);
  store_shader_uniform(post, primary_light_direction);
  store_shader_uniform(post, scan_time);
  store_shader_uniform(post, player_position);
  store_shader_uniform(post, astro_time_warp);
  store_shader_uniform(post, astro_time_warp_start_radius);
  store_shader_uniform(post, astro_time_warp_end_radius);
  store_shader_uniform(post, vignette_intensity);

  store_shader_uniform(surface, offset_and_scale);
  store_shader_uniform(surface, rotation);
  store_shader_uniform(surface, color);
  store_shader_uniform(surface, background);

  store_shader_uniform(debug_view, offset_and_scale);
  store_shader_uniform(debug_view, rotation);
  store_shader_uniform(debug_view, in_normal_and_depth);
  store_shader_uniform(debug_view, in_color_and_material);
  store_shader_uniform(debug_view, inverse_projection_matrix);
  store_shader_uniform(debug_view, inverse_view_matrix);
}

static void InitVertexFragmentShader(tOpenGLShader& shader, const char* vertex_path, const char* fragment_path) {
  shader.program = glCreateProgram();

  shader.attachments.push_back(
    AttachShader(shader, GL_VERTEX_SHADER, vertex_path)
  );

  shader.attachments.push_back(
    AttachShader(shader, GL_FRAGMENT_SHADER, fragment_path)
  );

  glLinkProgram(shader.program);

  shader_list.push_back(&shader);
}

static void InitVertexGeometryFragmentShader(tOpenGLShader& shader, const char* vertex_path, const char* geometry_path, const char* fragment_path) {
  // @todo
}

void Tachyon_OpenGL_InitShaders(tOpenGLShaders& shaders) {
  InitVertexFragmentShader(
    shaders.main_geometry,
    "./engine/opengl/shaders/main_geometry.vert.glsl",
    "./engine/opengl/shaders/main_geometry.frag.glsl"
  );

  InitVertexFragmentShader(
    shaders.shadow_map,
    "./engine/opengl/shaders/shadow_map.vert.glsl",
    "./engine/opengl/shaders/shadow_map.frag.glsl"
  );

  InitVertexFragmentShader(
    shaders.surface,
    "./engine/opengl/shaders/screen_quad.vert.glsl",
    "./engine/opengl/shaders/surface.frag.glsl"
  );

  InitVertexFragmentShader(
    shaders.global_lighting,
    "./engine/opengl/shaders/screen_quad.vert.glsl",
    "./engine/opengl/shaders/global_lighting.frag.glsl"
  );

  InitVertexFragmentShader(
    shaders.point_lights,
    "./engine/opengl/shaders/point_lights.vert.glsl",
    "./engine/opengl/shaders/point_lights.frag.glsl"
  );

  InitVertexFragmentShader(
    shaders.wireframe_mesh,
    "./engine/opengl/shaders/main_geometry.vert.glsl",
    "./engine/opengl/shaders/wireframe_mesh.frag.glsl"
  );

  InitVertexFragmentShader(
    shaders.volumetric_mesh,
    "./engine/opengl/shaders/post_geometry.vert.glsl",
    "./engine/opengl/shaders/volumetric_mesh.frag.glsl"
  );

  InitVertexFragmentShader(
    shaders.fire_mesh,
    "./engine/opengl/shaders/post_geometry.vert.glsl",
    "./engine/opengl/shaders/fire_mesh.frag.glsl"
  );

  InitVertexFragmentShader(
    shaders.ion_thruster_mesh,
    "./engine/opengl/shaders/post_geometry.vert.glsl",
    "./engine/opengl/shaders/ion_thruster_mesh.frag.glsl"
  );

  InitVertexFragmentShader(
    shaders.water_mesh,
    "./engine/opengl/shaders/post_geometry.vert.glsl",
    "./engine/opengl/shaders/water_mesh.frag.glsl"
  );

  InitVertexFragmentShader(
    shaders.post,
    "./engine/opengl/shaders/screen_quad.vert.glsl",
    "./engine/opengl/shaders/post.frag.glsl"
  );

  // @todo dev mode only
  {
    InitVertexFragmentShader(
      shaders.debug_view,
      "./engine/opengl/shaders/screen_quad.vert.glsl",
      "./engine/opengl/shaders/debug_view.frag.glsl"
    );
  }

  StoreShaderUniforms(shaders);
}

void Tachyon_OpenGL_HotReloadShaders(tOpenGLShaders& shaders) {
  for (auto& record : source_file_records) {
    auto full_path = std::filesystem::current_path() / record.path;
    auto last_write_time = std::filesystem::last_write_time(full_path);

    if (last_write_time != record.last_write_time) {
      changed_source_paths.push_back(record.path);

      record.last_write_time = last_write_time;
    }
  }

  if (changed_source_paths.size() > 0) {
    for (auto& changed_path : changed_source_paths) {
      for (auto* shader : shader_list) {
        for (auto& attachment : shader->attachments) {
          bool should_hot_reload = false;

          for (auto& dependency : attachment.dependencies) {
            if (dependency == changed_path) {
              should_hot_reload = true;

              break;
            }
          }

          if (should_hot_reload) {
            glDetachShader(shader->program, attachment.id);
            glDeleteShader(attachment.id);

            attachment = AttachShader(*shader, attachment.type, attachment.source_path.c_str());

            glLinkProgram(shader->program);

            add_console_message("Hot reloaded shader: " + attachment.source_path, tVec3f(1.f));
          }
        }
      }
    }

    // Update all shader uniforms. A single shader change will cause
    // all uniform locations across all shaders to be reloaded,
    // but in practice this is fast enough not to matter.
    StoreShaderUniforms(shaders);
  }

  changed_source_paths.clear();
}

void Tachyon_OpenGL_DestroyShaders(tOpenGLShaders& shaders) {
  // @todo
}