#include <chrono>
#include <map>

#include <glew.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_ttf.h>

#include "engine/tachyon_aliases.h"
#include "engine/tachyon_file_helpers.h"
#include "engine/tachyon_input.h"
#include "engine/tachyon_linear_algebra.h"
#include "engine/tachyon_timer.h"
#include "engine/opengl/tachyon_opengl_geometry.h"
#include "engine/opengl/tachyon_opengl_renderer.h"

#define get_renderer() (*(tOpenGLRenderer*)tachyon->renderer)

constexpr static uint32 INTERNAL_WIDTH = 1920;
constexpr static uint32 INTERNAL_HEIGHT = 1080;

internal void Tachyon_CheckError(const std::string& message) {
  GLenum error;

  static std::map<GLenum, std::string> glErrorMap = {
    { GL_INVALID_ENUM, "GL_INVALID_ENUM "},
    { GL_INVALID_VALUE, "GL_INVALID_VALUE" },
    { GL_INVALID_OPERATION, "GL_INVALID_OPERATION" },
    { GL_STACK_OVERFLOW, "GL_STACK_OVERFLOW" },
    { GL_STACK_UNDERFLOW, "GL_STACK_UNDERFLOW" },
    { GL_OUT_OF_MEMORY, "GL_OUT_OF_MEMORY" },
    { GL_INVALID_FRAMEBUFFER_OPERATION, "GL_INVALID_FRAMEBUFFER_OPERATION" },
    { GL_CONTEXT_LOST, "GL_CONTEXT_LOST" },
    { GL_TABLE_TOO_LARGE, "GL_TABLE_TOO_LARGE" }
  };

  while ((error = glGetError()) != GL_NO_ERROR) {
    printf("Error (%s): %s\n", message.c_str(), glErrorMap[error]);
  }
}

// --------------------------------------
// @todo move to tachyon_opengl_shaders.h
internal GLuint Tachyon_CreateShader(GLenum type, const char* path) {
  auto shader = glCreateShader(type);
  auto source = Tachyon_GetFileContents(path);
  auto* sourcePointer = source.c_str();

  glShaderSource(shader, 1, &sourcePointer, 0);
  glCompileShader(shader);

  GLint status;

  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

  if (status != GL_TRUE) {
    char error[512];

    glGetShaderInfoLog(shader, 512, 0, error);

    printf("Failed to compile shader: %s\n", path);
    printf("%s\n", error);
  }

  return shader;
}

internal void Tachyon_SetupShaders(tOpenGLShaders& shaders) {
  shaders.main_geometry.program = glCreateProgram();
  shaders.main_geometry.vertex_shader = Tachyon_CreateShader(GL_VERTEX_SHADER, "./engine/opengl/shaders/main_geometry.vert.glsl");
  shaders.main_geometry.fragment_shader = Tachyon_CreateShader(GL_FRAGMENT_SHADER, "./engine/opengl/shaders/main_geometry.frag.glsl");

  glAttachShader(shaders.main_geometry.program, shaders.main_geometry.vertex_shader);
  glAttachShader(shaders.main_geometry.program, shaders.main_geometry.fragment_shader);
  glLinkProgram(shaders.main_geometry.program);

  shaders.surface.program = glCreateProgram();
  shaders.surface.vertex_shader = Tachyon_CreateShader(GL_VERTEX_SHADER, "./engine/opengl/shaders/screen_quad.vert.glsl");
  shaders.surface.fragment_shader = Tachyon_CreateShader(GL_FRAGMENT_SHADER, "./engine/opengl/shaders/surface.frag.glsl");

  glAttachShader(shaders.surface.program, shaders.surface.vertex_shader);
  glAttachShader(shaders.surface.program, shaders.surface.fragment_shader);
  glLinkProgram(shaders.surface.program);

  shaders.sky_and_directional_lighting.program = glCreateProgram();
  shaders.sky_and_directional_lighting.vertex_shader = Tachyon_CreateShader(GL_VERTEX_SHADER, "./engine/opengl/shaders/screen_quad.vert.glsl");
  shaders.sky_and_directional_lighting.fragment_shader = Tachyon_CreateShader(GL_FRAGMENT_SHADER, "./engine/opengl/shaders/sky_and_directional_lighting.frag.glsl");

  glAttachShader(shaders.sky_and_directional_lighting.program, shaders.sky_and_directional_lighting.vertex_shader);
  glAttachShader(shaders.sky_and_directional_lighting.program, shaders.sky_and_directional_lighting.fragment_shader);
  glLinkProgram(shaders.sky_and_directional_lighting.program);

  // @todo dev mode only
  {
    shaders.debug_view.program = glCreateProgram();
    shaders.debug_view.vertex_shader = Tachyon_CreateShader(GL_VERTEX_SHADER, "./engine/opengl/shaders/screen_quad.vert.glsl");
    shaders.debug_view.fragment_shader = Tachyon_CreateShader(GL_FRAGMENT_SHADER, "./engine/opengl/shaders/debug_view.frag.glsl");

    glAttachShader(shaders.debug_view.program, shaders.debug_view.vertex_shader);
    glAttachShader(shaders.debug_view.program, shaders.debug_view.fragment_shader);
    glLinkProgram(shaders.debug_view.program);
  }
}

internal void Tachyon_UseShader(tOpenGLShader& shader) {
  glUseProgram(shader.program);
}

internal void Tachyon_SetShaderInt(tOpenGLShader& shader, const std::string& name, const int value) {
  GLint location = glGetUniformLocation(shader.program, name.c_str());

  glUniform1i(location, value);
}

internal void Tachyon_SetShaderVec3f(tOpenGLShader& shader, const std::string& name, const tVec3f& vector) {
  GLint location = glGetUniformLocation(shader.program, name.c_str());

  glUniform3fv(location, 1, &vector.x);
}

internal void Tachyon_SetShaderVec4f(tOpenGLShader& shader, const std::string& name, const tVec4f& vector) {
  GLint location = glGetUniformLocation(shader.program, name.c_str());

  glUniform4fv(location, 1, &vector.x);
}

internal void Tachyon_SetShaderMat4f(tOpenGLShader& shader, const std::string& name, const tMat4f& matrix) {
  GLint location = glGetUniformLocation(shader.program, name.c_str());

  glUniformMatrix4fv(location, 1, GL_FALSE, matrix.m);
}
// --------------------------------------

internal void RenderScreenQuad(Tachyon* tachyon) {
  auto& renderer = get_renderer();
  auto& quad = renderer.screen_quad;

  glBindVertexArray(quad.vao);
  glBindBuffer(GL_ARRAY_BUFFER, quad.vbo);
  glDrawArrays(GL_TRIANGLES, 0, 6);
}

internal void RenderSurface(Tachyon* tachyon, SDL_Surface* surface, uint32 x, uint32 y, uint32 w, uint32 h, const tVec3f& color, const tVec4f& background) {
  auto& renderer = get_renderer();
  auto& ctx = renderer.ctx;
  auto& shaders = renderer.shaders;

  float offsetX = -1.0f + (2 * x + w) / (float)ctx.w;
  float offsetY = 1.0f - (2 * y + h) / (float)ctx.h;
  float scaleX = w / (float)ctx.w;
  float scaleY = -1.0f * h / (float)ctx.h;
  int format = surface->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB;

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, renderer.screen_quad_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, format, surface->w, surface->h, 0, format, GL_UNSIGNED_BYTE, surface->pixels);

  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  Tachyon_UseShader(shaders.surface);
  Tachyon_SetShaderVec4f(shaders.surface, "transform", { offsetX, offsetY, scaleX, scaleY });
  Tachyon_SetShaderVec3f(shaders.surface, "color", color);
  Tachyon_SetShaderVec4f(shaders.surface, "background", background);

  RenderScreenQuad(tachyon);
}

internal void RenderText(Tachyon* tachyon, TTF_Font* font, const char* message, uint32 x, uint32 y, const tVec3f& color, const tVec4f& background) {
  SDL_Surface* text = TTF_RenderText_Blended_Wrapped(font, message, { 255, 255, 255 }, INTERNAL_WIDTH);

  RenderSurface(tachyon, text, x, y, text->w, text->h, color, background);

  SDL_FreeSurface(text);
}

internal void HandleDeveloperTools(Tachyon* tachyon) {
  auto& renderer = get_renderer();

  // Keyboard shortcuts
  {
    // Toggle the G-Buffer view with TAB
    if (did_press_key(tKey::TAB)) {
      renderer.show_debug_view = !renderer.show_debug_view;
    }
  }

  // Developer overlay
  // @todo load messages into a list and render everything in a loop
  {
    auto view_label = "View: " + std::string(
      renderer.show_debug_view ? "DEBUG" : "DEFAULT"
    );

    RenderText(tachyon, tachyon->developer_overlay_font, view_label.c_str(), 10, 10, tVec3f(1.f), tVec4f(0.f));

    auto runtime_label = "Running time: " + std::to_string(tachyon->running_time);

    RenderText(tachyon, tachyon->developer_overlay_font, runtime_label.c_str(), 10, 35, tVec3f(1.f), tVec4f(0.f));

    auto render_fps = uint32(1000000.f / (float)renderer.last_render_time_in_microseconds);
    auto render_fps_label = "Render time: " + std::to_string(renderer.last_render_time_in_microseconds) + "us (" + std::to_string(render_fps) + "fps)";

    RenderText(tachyon, tachyon->developer_overlay_font, render_fps_label.c_str(), 10, 60, tVec3f(1.f), tVec4f(0.f));

    auto frame_fps = uint32(1000000.f / (float)tachyon->last_frame_time_in_microseconds);
    auto frame_fps_label = "Frame time: " + std::to_string(tachyon->last_frame_time_in_microseconds) + "us (" + std::to_string(frame_fps) + "fps)";

    RenderText(tachyon, tachyon->developer_overlay_font, frame_fps_label.c_str(), 10, 85, tVec3f(1.f), tVec4f(0.f));
  }
}

internal void UpdateRendererContext(Tachyon* tachyon) {
  auto& ctx = get_renderer().ctx;
  auto& camera = tachyon->scene.camera;
  int w, h;

  SDL_GL_GetDrawableSize(tachyon->sdl_window, &w, &h);

  ctx.w = w;
  ctx.h = h;

  ctx.projection_matrix = tMat4f::perspective(45.f, 1.f, 10000.f);

  ctx.view_matrix = (
    camera.orientation.toQuaternion().toMatrix4f() *
    tMat4f::translation(camera.position * tVec3f(-1.f))
  );

  // @todo set inv proj + inv view matrices
}

internal void RenderStaticGeometry(Tachyon* tachyon) {
  auto& renderer = get_renderer();
  auto& ctx = renderer.ctx;
  auto& glPack = renderer.mesh_pack;

  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);

  renderer.g_buffer.write();

  glClearColor(0.f, 0.f, 0.f, 1.f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  tMat4f mat_view_projection = (ctx.projection_matrix * ctx.view_matrix).transpose();

  Tachyon_UseShader(renderer.shaders.main_geometry);
  Tachyon_SetShaderMat4f(renderer.shaders.main_geometry, "mat_view_projection", mat_view_projection);

  glBindVertexArray(glPack.vao);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glPack.ebo);

  // @temporary
  // @todo buffer sub data per updated mesh record/object group
  {
    // Buffer colors
    auto& colors = tachyon->colors;
    glBindBuffer(GL_ARRAY_BUFFER, glPack.buffers[COLOR_BUFFER]);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(tVec4f), colors.data(), GL_DYNAMIC_DRAW);

    // Buffer matrices
    auto& matrices = tachyon->matrices;
    glBindBuffer(GL_ARRAY_BUFFER, glPack.buffers[MATRIX_BUFFER]);
    glBufferData(GL_ARRAY_BUFFER, matrices.size() * sizeof(tMat4f), matrices.data(), GL_DYNAMIC_DRAW);
  }

  for (auto& record : tachyon->mesh_pack.mesh_records) {
    // @todo do the matrix/color buffering here

    record.group.buffered = true;
  }

  struct DrawElementsIndirectCommand {
    GLuint count;
    GLuint instanceCount;
    GLuint firstIndex;
    GLuint baseVertex;
    GLuint baseInstance;
  };

  auto& records = tachyon->mesh_pack.mesh_records;
  auto totalMeshes = records.size();
  auto* commands = new DrawElementsIndirectCommand[totalMeshes];

  for (uint32 i = 0; i < totalMeshes; i++) {
    auto& command = commands[i];
    auto& record = records[i];

    command.count = record.face_element_end - record.face_element_start;
    command.firstIndex = record.face_element_start;
    command.instanceCount = record.group.total_visible;
    command.baseInstance = record.group.object_offset;
    command.baseVertex = record.vertex_start;
  }

  glBindBuffer(GL_DRAW_INDIRECT_BUFFER, renderer.indirect_buffer);
  glBufferData(GL_DRAW_INDIRECT_BUFFER, totalMeshes * sizeof(DrawElementsIndirectCommand), commands, GL_DYNAMIC_DRAW);

  glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0, totalMeshes, 0);

  delete[] commands;
}

internal void RenderDebugView(Tachyon* tachyon) {
  auto& renderer = get_renderer();
  auto& shaders = renderer.shaders;
  auto& ctx = renderer.ctx;

  renderer.g_buffer.read();

  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glViewport(0, 0, ctx.w, ctx.h);

  Tachyon_UseShader(shaders.debug_view);
  Tachyon_SetShaderVec4f(shaders.debug_view, "transform", { 0.f, 0.f, 1.f, 1.f });
  Tachyon_SetShaderInt(shaders.debug_view, "in_color_and_depth", 0);
  Tachyon_SetShaderInt(shaders.debug_view, "in_normal_and_material", 1);

  RenderScreenQuad(tachyon);
}

internal void RenderSkyAndDirectionalLighting(Tachyon* tachyon) {
  auto& renderer = get_renderer();
  auto& shaders = renderer.shaders;
  auto& ctx = renderer.ctx;

  renderer.g_buffer.read();

  // @todo render to an accumulation buffer
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glViewport(0, 0, ctx.w, ctx.h);

  Tachyon_UseShader(shaders.sky_and_directional_lighting);
  Tachyon_SetShaderVec4f(shaders.sky_and_directional_lighting, "transform", { 0.f, 0.f, 1.f, 1.f });
  Tachyon_SetShaderInt(shaders.sky_and_directional_lighting, "in_color_and_depth", 0);
  Tachyon_SetShaderInt(shaders.sky_and_directional_lighting, "in_normal_and_material", 1);

  RenderScreenQuad(tachyon);
}

void Tachyon_InitOpenGLRenderer(Tachyon* tachyon) {
  auto* renderer = new tOpenGLRenderer;

  // @todo only do GL setup stuff once in case we destroy/recreate the renderer
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

  renderer->gl_context = SDL_GL_CreateContext(tachyon->sdl_window);

  glewExperimental = true;

  glewInit();

  SDL_GL_SetSwapInterval(0);

  // Apply default OpenGL settings
  glEnable(GL_PROGRAM_POINT_SIZE);
  glFrontFace(GL_CW);

  Tachyon_SetupShaders(renderer->shaders);

  // @todo Tachyon_SetupBuffers()
  {
    glGenBuffers(1, &renderer->indirect_buffer);

    auto& g_buffer = renderer->g_buffer;

    g_buffer.init();
    g_buffer.setSize(INTERNAL_WIDTH, INTERNAL_HEIGHT);
    g_buffer.addColorAttachment(ColorFormat::RGBA);
    g_buffer.addColorAttachment(ColorFormat::RGBA16);
    g_buffer.addDepthStencilAttachment();
    g_buffer.bindColorAttachments();
  }

  // Set up screen quad texture binding
  {
    glGenTextures(1, &renderer->screen_quad_texture);
    glBindTexture(GL_TEXTURE_2D, renderer->screen_quad_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }

  // Create additional resources
  {
    renderer->mesh_pack = Tachyon_CreateOpenGLMeshPack(tachyon);
    renderer->screen_quad = Tachyon_CreateOpenGLScreenQuad(tachyon);
  }

  tachyon->renderer = renderer;
}

void Tachyon_RenderSceneInOpenGL(Tachyon* tachyon) {
  auto& renderer = get_renderer();
  auto& ctx = renderer.ctx;
  auto start = Tachyon_GetMicroseconds();

  UpdateRendererContext(tachyon);
  RenderStaticGeometry(tachyon);

  // The next steps in the pipeline render quads in screen space,
  // so we don't need to do any back-face culling or depth testing
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);

  if (renderer.show_debug_view) {
    RenderDebugView(tachyon);
  } else {
    RenderSkyAndDirectionalLighting(tachyon);
  }

  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glViewport(0, 0, ctx.w, ctx.h);

  // @todo dev mode only
  HandleDeveloperTools(tachyon);

  SDL_GL_SwapWindow(tachyon->sdl_window);

  renderer.last_render_time_in_microseconds = Tachyon_GetMicroseconds() - start;
}

void Tachyon_DestroyOpenGLRenderer(Tachyon* tachyon) {
  auto& renderer = get_renderer();

  glDeleteBuffers(1, &renderer.indirect_buffer);
  // @todo destroy shaders/buffers/etc.

  SDL_GL_DeleteContext(renderer.gl_context);
}