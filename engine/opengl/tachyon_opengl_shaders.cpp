#include "engine/tachyon_aliases.h"
#include "engine/tachyon_file_helpers.h"
#include "engine/opengl/tachyon_opengl_shaders.h"

static GLuint CreateShader(GLenum type, const char* path) {
  auto shader = glCreateShader(type);
  auto source = Tachyon_GetFileContents(path);
  auto* sourcePointer = source.c_str();

  glShaderSource(shader, 1, &sourcePointer, 0);
  glCompileShader(shader);

  // @todo dev mode only
  {
    GLint status;

    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    if (status != GL_TRUE) {
      char error[512];

      glGetShaderInfoLog(shader, 512, 0, error);

      // @todo print to game window
      printf("Failed to compile shader: %s\n", path);
      printf("%s\n", error);
    }
  }

  return shader;
}

static void StoreShaderUniforms(tOpenGLShaders& shaders) {
  #define store_shader_uniform(shader_name, uniform_name) \
    shaders.locations.shader_name.uniform_name = glGetUniformLocation(shaders.shader_name.program, #uniform_name);

  store_shader_uniform(main_geometry, mat_view_projection);

  store_shader_uniform(surface, transform);
  store_shader_uniform(surface, color);
  store_shader_uniform(surface, background);

  store_shader_uniform(sky_and_directional_lighting, transform);
  store_shader_uniform(sky_and_directional_lighting, in_color_and_depth);
  store_shader_uniform(sky_and_directional_lighting, in_normal_and_material);
  store_shader_uniform(sky_and_directional_lighting, inverse_projection_matrix);
  store_shader_uniform(sky_and_directional_lighting, inverse_view_matrix);

  store_shader_uniform(debug_view, transform);
  store_shader_uniform(debug_view, in_color_and_depth);
  store_shader_uniform(debug_view, in_normal_and_material);
  store_shader_uniform(debug_view, inverse_projection_matrix);
  store_shader_uniform(debug_view, inverse_view_matrix);
}

static void InitVertexFragmentShader(tOpenGLShader& shader, const char* vertex_path, const char* fragment_path) {
  shader.program = glCreateProgram();
  shader.vertex_shader = CreateShader(GL_VERTEX_SHADER, vertex_path);
  shader.fragment_shader = CreateShader(GL_FRAGMENT_SHADER, fragment_path);

  glAttachShader(shader.program, shader.vertex_shader);
  glAttachShader(shader.program, shader.fragment_shader);
  glLinkProgram(shader.program);
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
  // @todo
}

void Tachyon_OpenGL_DestroyShaders(tOpenGLShaders& shaders) {
  // @todo
}