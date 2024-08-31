#include <filesystem>
#include <string>
#include <vector>

#include "engine/tachyon_aliases.h"
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
  auto source = Tachyon_GetFileContents(path);
  auto* source_pointer = source.c_str();

  // @todo handle #includes/other directives etc.

  SaveSourceFileRecord(path);

  glShaderSource(id, 1, &source_pointer, 0);
  glCompileShader(id);

  // @todo dev mode only
  {
    GLint status;

    glGetShaderiv(id, GL_COMPILE_STATUS, &status);

    if (status != GL_TRUE) {
      char error[512];

      glGetShaderInfoLog(id, 512, 0, error);

      // @todo print to game window
      printf("Failed to compile shader: %s\n", path);
      printf("\u001b[31m %s\n\u001b[37m", error);
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

  store_shader_uniform(main_geometry, mat_view_projection);

  store_shader_uniform(surface, transform);
  store_shader_uniform(surface, color);
  store_shader_uniform(surface, background);

  store_shader_uniform(sky_and_directional_lighting, transform);
  store_shader_uniform(sky_and_directional_lighting, in_normal_and_depth);
  store_shader_uniform(sky_and_directional_lighting, in_color_and_material);
  store_shader_uniform(sky_and_directional_lighting, inverse_projection_matrix);
  store_shader_uniform(sky_and_directional_lighting, inverse_view_matrix);
  store_shader_uniform(sky_and_directional_lighting, camera_position);

  store_shader_uniform(debug_view, transform);
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
    shaders.surface,
    "./engine/opengl/shaders/screen_quad.vert.glsl",
    "./engine/opengl/shaders/surface.frag.glsl"
  );

  InitVertexFragmentShader(
    shaders.sky_and_directional_lighting,
    "./engine/opengl/shaders/screen_quad.vert.glsl",
    "./engine/opengl/shaders/sky_and_directional_lighting.frag.glsl"
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

            printf("Hot reloaded: %s\n", attachment.source_path);
          }
        }
      }
    }

    // Doing this means a single shader change will cause
    // all uniform locations across all shaders to be reloaded,
    // but in practice this is fast enough not to matter.
    StoreShaderUniforms(shaders);
  }

  changed_source_paths.clear();
}

void Tachyon_OpenGL_DestroyShaders(tOpenGLShaders& shaders) {
  // @todo
}