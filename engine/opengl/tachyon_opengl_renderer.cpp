#include <chrono>
#include <map>

#include <glew.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_ttf.h>

#include "engine/tachyon_aliases.h"
#include "engine/tachyon_console.h"
#include "engine/tachyon_file_helpers.h"
#include "engine/tachyon_input.h"
#include "engine/tachyon_linear_algebra.h"
#include "engine/tachyon_timer.h"
#include "engine/opengl/tachyon_opengl_geometry.h"
#include "engine/opengl/tachyon_opengl_renderer.h"

#define get_renderer() (*(tOpenGLRenderer*)tachyon->renderer)

constexpr static uint32 INTERNAL_WIDTH = 1920;
constexpr static uint32 INTERNAL_HEIGHT = 1080;

static void Tachyon_CheckError(const std::string& message) {
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
static void Tachyon_SetShaderInt(GLint location, const int value) {
  glUniform1i(location, value);
}

static void Tachyon_SetShaderVec3f(GLint location, const tVec3f& vector) {
  glUniform3fv(location, 1, &vector.x);
}

static void Tachyon_SetShaderVec4f(GLint location, const tVec4f& vector) {
  glUniform4fv(location, 1, &vector.x);
}

static void Tachyon_SetShaderMat4f(GLint location, const tMat4f& matrix) {
  glUniformMatrix4fv(location, 1, GL_FALSE, matrix.m);
}

static void Tachyon_SetShaderMat4f(GLuint location, const tMat4f& matrix) {
  glUniformMatrix4fv(location, 1, GL_FALSE, matrix.m);
}
// --------------------------------------

static void RenderScreenQuad(Tachyon* tachyon) {
  auto& renderer = get_renderer();
  auto& quad = renderer.screen_quad;

  glBindVertexArray(quad.vao);
  glBindBuffer(GL_ARRAY_BUFFER, quad.vbo);
  glDrawArrays(GL_TRIANGLES, 0, 6);

  // @todo dev mode only
  {
    renderer.total_draw_calls += 1;
  }
}

static void RenderSurface(Tachyon* tachyon, SDL_Surface* surface, uint32 x, uint32 y, uint32 w, uint32 h, const tVec3f& color, const tVec4f& background) {
  auto& renderer = get_renderer();
  auto& ctx = renderer.ctx;
  auto& shader = renderer.shaders.surface;
  auto& locations = renderer.shaders.locations.surface;

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

  glUseProgram(shader.program);
  Tachyon_SetShaderVec4f(locations.transform, { offsetX, offsetY, scaleX, scaleY });
  Tachyon_SetShaderVec3f(locations.color, color);
  Tachyon_SetShaderVec4f(locations.background, background);

  RenderScreenQuad(tachyon);
}

static void RenderText(Tachyon* tachyon, TTF_Font* font, const char* message, uint32 x, uint32 y, const tVec3f& color, const tVec4f& background) {
  SDL_Surface* text = TTF_RenderText_Blended_Wrapped(font, message, { 255, 255, 255 }, INTERNAL_WIDTH);

  RenderSurface(tachyon, text, x, y, text->w, text->h, color, background);

  SDL_FreeSurface(text);
}

static void HandleDeveloperTools(Tachyon* tachyon) {
  auto& renderer = get_renderer();

  // Keyboard shortcuts
  {
    // Toggle the G-Buffer view with TAB
    if (did_press_key(tKey::TAB)) {
      renderer.show_g_buffer_view = !renderer.show_g_buffer_view;
    }

    if (did_press_key(tKey::V)) {
      auto swap_interval = SDL_GL_GetSwapInterval();

      SDL_GL_SetSwapInterval(swap_interval ? 0 : 1);
    }
  }

  // Developer overlay
  {
    #define String(v) std::to_string(v)

    auto render_fps = uint32(1000000.f / (float)renderer.last_render_time_in_microseconds);
    auto frame_fps = uint32(1000000.f / (float)tachyon->last_frame_time_in_microseconds);

    std::vector<std::string> labels = {
      "View: " + std::string(renderer.show_g_buffer_view ? "G-BUFFER" : "DEFAULT"),
      "Running time: " + String(tachyon->running_time),
      "Render time: " + String(renderer.last_render_time_in_microseconds) + "us (" + String(render_fps) + "fps)",
      "Frame time: " + String(tachyon->last_frame_time_in_microseconds) + "us (" + String(frame_fps) + "fps)",
      "Triangles: " + String(renderer.total_triangles),
      "Vertices: " + String(renderer.total_vertices),
      "Draw calls: " + String(renderer.total_draw_calls)
    };

    // Engine labels
    uint32 y_offset = 10;

    for (auto& label : labels) {
      RenderText(tachyon, tachyon->developer_overlay_font, label.c_str(), 10, y_offset, tVec3f(1.f), tVec4f(0.f));

      y_offset += 25;
    }

    y_offset += 25;

    // Custom dev labels
    for (auto& dev_label : tachyon->dev_labels) {
      auto full_label = dev_label.label + ": " + dev_label.message;

      RenderText(tachyon, tachyon->developer_overlay_font, full_label.c_str(), 10, y_offset, tVec3f(1.f), tVec4f(0.2f, 0.2f, 1.f, 0.4));

      y_offset += 30;
    }
  }

  // Console messages
  {
    auto& console_messages = Tachyon_GetConsoleMessages();
    uint32 y_offset = renderer.ctx.h - console_messages.size() * 30 - 10;

    for (auto& console_message : console_messages) {
      auto& message = console_message.message;
      auto& color = console_message.color;

      RenderText(tachyon, tachyon->developer_overlay_font, message.c_str(), 10, y_offset, color, tVec4f(0.f));

      y_offset += 30;
    }
  }
}

static void UpdateRendererContext(Tachyon* tachyon) {
  auto& ctx = get_renderer().ctx;
  auto& camera = tachyon->scene.camera;
  int w, h;

  SDL_GL_GetDrawableSize(tachyon->sdl_window, &w, &h);

  ctx.w = w;
  ctx.h = h;

  // @todo make fov/near/far customizable
  ctx.projection_matrix = tMat4f::perspective(45.f, 100.f, 10000000.f).transpose();

  ctx.view_matrix = (
    camera.rotation.toMatrix4f() *
    tMat4f::translation(camera.position * tVec3f(-1.f))
  ).transpose();

  ctx.inverse_projection_matrix = ctx.projection_matrix.inverse();
  ctx.inverse_view_matrix = ctx.view_matrix.inverse();

  ctx.camera_position = camera.position;
}

static void RenderStaticGeometry(Tachyon* tachyon) {
  auto& renderer = get_renderer();
  auto& shader = renderer.shaders.main_geometry;
  auto& locations = renderer.shaders.locations.main_geometry;
  auto& ctx = renderer.ctx;
  auto& gl_mesh_pack = renderer.mesh_pack;

  // @todo have a separate method for this
  for (auto& record : tachyon->mesh_pack.mesh_records) {
    if (!record.group.buffered) {
      glBindBuffer(GL_ARRAY_BUFFER, gl_mesh_pack.buffers[SURFACE_BUFFER]);
      glBufferSubData(GL_ARRAY_BUFFER, record.group.object_offset * sizeof(uint32), record.group.total_visible * sizeof(uint32), record.group.surfaces);

      glBindBuffer(GL_ARRAY_BUFFER, gl_mesh_pack.buffers[MATRIX_BUFFER]);
      glBufferSubData(GL_ARRAY_BUFFER, record.group.object_offset * sizeof(tMat4f), record.group.total_visible * sizeof(tMat4f), record.group.matrices);
    }

    record.group.buffered = true;
  }

  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);

  renderer.g_buffer.write();

  glClearColor(0.f, 0.f, 0.f, 1.f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  tMat4f mat_view_projection = ctx.view_matrix * ctx.projection_matrix;

  glUseProgram(shader.program);
  Tachyon_SetShaderMat4f(locations.mat_view_projection, mat_view_projection);

  glBindVertexArray(gl_mesh_pack.vao);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_mesh_pack.ebo);

  struct DrawElementsIndirectCommand {
    GLuint count;
    GLuint instanceCount;
    GLuint firstIndex;
    GLuint baseVertex;
    GLuint baseInstance;
  };

  auto& records = tachyon->mesh_pack.mesh_records;
  auto totalMeshes = records.size();
  // @todo avoid allocating + deleting each frame
  auto* commands = new DrawElementsIndirectCommand[totalMeshes];

  for (uint32 i = 0; i < totalMeshes; i++) {
    auto& command = commands[i];
    auto& record = records[i];

    command.count = record.face_element_end - record.face_element_start;
    command.firstIndex = record.face_element_start;
    command.instanceCount = record.group.total_visible;
    command.baseInstance = record.group.object_offset;
    command.baseVertex = record.vertex_start;

    // @todo dev mode only
    {
      renderer.total_triangles += command.count * command.instanceCount;
      renderer.total_vertices += (record.vertex_end - record.vertex_start) * command.instanceCount;
    }
  }

  glBindBuffer(GL_DRAW_INDIRECT_BUFFER, renderer.indirect_buffer);
  glBufferData(GL_DRAW_INDIRECT_BUFFER, totalMeshes * sizeof(DrawElementsIndirectCommand), commands, GL_DYNAMIC_DRAW);

  glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0, totalMeshes, 0);

  delete[] commands;

  // @todo dev mode only
  {
    renderer.total_draw_calls += 1;
  }
}

static void RenderGBufferView(Tachyon* tachyon) {
  auto& renderer = get_renderer();
  auto& shader = renderer.shaders.debug_view;
  auto& locations = renderer.shaders.locations.debug_view;
  auto& ctx = renderer.ctx;

  renderer.g_buffer.read();

  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glViewport(0, 0, ctx.w, ctx.h);

  glUseProgram(shader.program);
  Tachyon_SetShaderVec4f(locations.transform, { 0.f, 0.f, 1.f, 1.f });
  Tachyon_SetShaderInt(locations.in_normal_and_depth, 0);
  Tachyon_SetShaderInt(locations.in_color_and_material, 1);
  Tachyon_SetShaderMat4f(locations.inverse_projection_matrix, ctx.inverse_projection_matrix);
  Tachyon_SetShaderMat4f(locations.inverse_view_matrix, ctx.inverse_view_matrix);

  RenderScreenQuad(tachyon);
}

static void RenderSkyAndDirectionalLighting(Tachyon* tachyon) {
  auto& renderer = get_renderer();
  auto& shader = renderer.shaders.sky_and_directional_lighting;
  auto& locations = renderer.shaders.locations.sky_and_directional_lighting;
  auto& ctx = renderer.ctx;

  renderer.g_buffer.read();

  // @todo render to an accumulation buffer
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glViewport(0, 0, ctx.w, ctx.h);

  glUseProgram(shader.program);
  Tachyon_SetShaderVec4f(locations.transform, { 0.f, 0.f, 1.f, 1.f });
  Tachyon_SetShaderInt(locations.in_normal_and_depth, 0);
  Tachyon_SetShaderInt(locations.in_color_and_material, 1);
  Tachyon_SetShaderMat4f(locations.inverse_projection_matrix, ctx.inverse_projection_matrix);
  Tachyon_SetShaderMat4f(locations.inverse_view_matrix, ctx.inverse_view_matrix);
  Tachyon_SetShaderVec3f(locations.camera_position, ctx.camera_position);
  // @temporary
  // @todo allow multiple directional lights
  Tachyon_SetShaderVec3f(locations.directional_light_direction, tachyon->scene.directional_light_direction);

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
  glFrontFace(GL_CCW);

  Tachyon_OpenGL_InitShaders(renderer->shaders);

  // @todo InitBuffers()
  {
    glGenBuffers(1, &renderer->indirect_buffer);

    auto& g_buffer = renderer->g_buffer;

    g_buffer.init();
    g_buffer.setSize(INTERNAL_WIDTH, INTERNAL_HEIGHT);
    g_buffer.addColorAttachment(ColorFormat::RGBA);
    g_buffer.addColorAttachment(ColorFormat::RGBA8UI);
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

  {
    // Buffer surface data
    auto& surfaces = tachyon->surfaces;
    glBindBuffer(GL_ARRAY_BUFFER, renderer->mesh_pack.buffers[SURFACE_BUFFER]);
    glBufferData(GL_ARRAY_BUFFER, surfaces.size() * sizeof(uint32), surfaces.data(), GL_DYNAMIC_DRAW);

    // Buffer matrices
    auto& matrices = tachyon->matrices;
    glBindBuffer(GL_ARRAY_BUFFER, renderer->mesh_pack.buffers[MATRIX_BUFFER]);
    glBufferData(GL_ARRAY_BUFFER, matrices.size() * sizeof(tMat4f), matrices.data(), GL_DYNAMIC_DRAW);
  }

  tachyon->renderer = renderer;
}

void Tachyon_RenderSceneInOpenGL(Tachyon* tachyon) {
  auto& renderer = get_renderer();
  auto& ctx = renderer.ctx;
  auto start = Tachyon_GetMicroseconds();

  if (tachyon->running_time - renderer.last_shader_hot_reload_time > 1.f) {
    Tachyon_OpenGL_HotReloadShaders(renderer.shaders);

    renderer.last_shader_hot_reload_time = tachyon->running_time;
  }

  // @todo dev mode only
  {
    renderer.total_triangles = 0;
    renderer.total_vertices = 0;
    renderer.total_draw_calls = 0;
  }

  UpdateRendererContext(tachyon);
  RenderStaticGeometry(tachyon);

  // The next steps in the pipeline render quads in screen space,
  // so we don't need to do any back-face culling or depth testing
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);

  if (renderer.show_g_buffer_view) {
    RenderGBufferView(tachyon);
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
  // @todo DestroyBuffers()

  Tachyon_OpenGL_DestroyShaders(renderer.shaders);

  SDL_GL_DeleteContext(renderer.gl_context);
}