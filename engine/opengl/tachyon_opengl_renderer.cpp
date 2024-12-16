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

enum Attachment {
  G_BUFFER_NORMALS_AND_DEPTH = 0,
  G_BUFFER_COLOR_AND_MATERIAL = 1,
  ACCUMULATION_COLOR_AND_DEPTH = 2,
  ACCUMULATION_TEMPORAL_DATA = 3,
  DIRECTIONAL_SHADOW_MAP_CASCADE_1 = 4,
  DIRECTIONAL_SHADOW_MAP_CASCADE_2 = 5,
  DIRECTIONAL_SHADOW_MAP_CASCADE_3 = 6,
  DIRECTIONAL_SHADOW_MAP_CASCADE_4 = 7
};

struct DrawElementsIndirectCommand {
  GLuint count;
  GLuint instanceCount;
  GLuint firstIndex;
  GLuint baseVertex;
  GLuint baseInstance;
};

// --------------------------------------
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
    printf("Error (%s): %s\n", message.c_str(), glErrorMap[error].c_str());
  }
}
// --------------------------------------
static void SetShaderBool(GLint location, const bool value) {
  glUniform1i(location, value);
}

static void SetShaderInt(GLint location, const int value) {
  glUniform1i(location, value);
}

static void SetShaderFloat(GLint location, const float value) {
  glUniform1f(location, value);
}

static void SetShaderVec3f(GLint location, const tVec3f& vector) {
  glUniform3fv(location, 1, &vector.x);
}

static void SetShaderVec4f(GLint location, const tVec4f& vector) {
  glUniform4fv(location, 1, &vector.x);
}

static void SetShaderMat4f(GLint location, const tMat4f& matrix) {
  glUniformMatrix4fv(location, 1, GL_FALSE, matrix.m);
}

static void SetShaderMat4f(GLuint location, const tMat4f& matrix) {
  glUniformMatrix4fv(location, 1, GL_FALSE, matrix.m);
}
// --------------------------------------
// { near, far }
const static float cascade_depth_ranges[4][2] = {
  { 500.0f, 10000.0f },
  { 10000.0f, 50000.0f },
  { 50000.0f, 100000.0f },
  { 100000.f, 500000.f }
};

/**
 * Adapted from https://alextardif.com/shadowmapping.html
 */
tMat4f CreateCascadedLightMatrix(uint8 cascade, const tVec3f& light_direction, const tCamera& camera) {
  // Determine the near and far ranges of the cascade volume
  float near = cascade_depth_ranges[cascade][0];
  float far = cascade_depth_ranges[cascade][1];

  // Define clip space camera frustum
  tVec3f corners[] = {
    tVec3f(-1.0f, 1.0f, -1.0f),   // Near plane, top left
    tVec3f(1.0f, 1.0f, -1.0f),    // Near plane, top right
    tVec3f(-1.0f, -1.0f, -1.0f),  // Near plane, bottom left
    tVec3f(1.0f, -1.0f, -1.0f),   // Near plane, bottom right

    tVec3f(-1.0f, 1.0f, 1.0f),    // Far plane, top left
    tVec3f(1.0f, 1.0f, 1.0f),     // Far plane, top right
    tVec3f(-1.0f, -1.0f, 1.0f),   // Far plane, bottom left
    tVec3f(1.0f, -1.0f, 1.0f)     // Far plane, bottom right
  };

  // Transform clip space camera frustum into world space
  tMat4f camera_view = (
    camera.rotation.toMatrix4f() *
    tMat4f::translation(camera.position.invert())
  );

  tMat4f camera_projection = tMat4f::perspective(camera.fov, near, far);
  tMat4f camera_view_projection = camera_projection * camera_view;
  tMat4f inverse_camera_view_projection = camera_view_projection.inverse();

  for (uint32 i = 0; i < 8; i++) {
    corners[i] = (inverse_camera_view_projection * tVec4f(corners[i], 1.f)).homogenize();
  }

  // Calculate world space frustum center/centroid
  tVec3f frustum_center;

  for (uint32 i = 0; i < 8; i++) {
    frustum_center += corners[i];
  }

  frustum_center /= 8.0f;

  // Calculate the radius of a sphere encapsulating the frustum
  float radius = 0.0f;

  for (uint32 i = 0; i < 8; i++) {
    radius = std::max(radius, (frustum_center - corners[i]).magnitude());
  }

  // Calculate the ideal frustum center, 'snapped' to the shadow map texel
  // grid to avoid warbling and other distortions when moving the camera
  float texels_per_unit = 2048.f / (radius * 2.f);

  // Determine the top (up) vector for the lookAt matrix
  bool is_vertical_light = light_direction == tVec3f(0, 1.f, 0) || light_direction == tVec3f(0, -1.f, 0);
  tVec3f up_vector = is_vertical_light ? tVec3f(0, 0, 1.f) : tVec3f(0, 1.f, 0);

  tMat4f texel_look_at_matrix = tMat4f::lookAt(tVec3f(0.0f), light_direction.invert(), up_vector);
  tMat4f texel_scale_matrix = tMat4f::scale(texels_per_unit);
  tMat4f texel_matrix = texel_scale_matrix * texel_look_at_matrix;

  // Align the frustum center in texel space, and then
  // restore that to its world space coordinates
  frustum_center = (texel_matrix * tVec4f(frustum_center, 1.f)).homogenize();
  frustum_center.x = floorf(frustum_center.x);
  frustum_center.y = floorf(frustum_center.y);
  frustum_center = (texel_matrix.inverse() * tVec4f(frustum_center, 1.f)).homogenize();

  // Compute final light view matrix for rendering the shadow map
  tMat4f projection_matrix = tMat4f::orthographic(radius, -radius, -radius, radius, -radius - 250000.0f, radius);
  tMat4f view_matrix = tMat4f::lookAt(frustum_center, light_direction.invert(), up_vector);

  return (projection_matrix * view_matrix).transpose();
}
// --------------------------------------
static void CreateRenderBuffers(Tachyon* tachyon) {
  auto& renderer = get_renderer();
  auto& g_buffer = renderer.g_buffer;
  auto& accumulation_buffer_a = renderer.accumulation_buffer_a;
  auto& accumulation_buffer_b = renderer.accumulation_buffer_b;
  auto& directional_shadow_map = renderer.directional_shadow_map;
  int w, h;

  SDL_GL_GetDrawableSize(tachyon->sdl_window, &w, &h);

  g_buffer.init();
  g_buffer.setSize(w, h);
  g_buffer.addColorAttachment(ColorFormat::RGBA, G_BUFFER_NORMALS_AND_DEPTH);
  g_buffer.addColorAttachment(ColorFormat::RGBA8UI, G_BUFFER_COLOR_AND_MATERIAL);
  g_buffer.addDepthStencilAttachment();
  g_buffer.bindColorAttachments();

  accumulation_buffer_a.init();
  accumulation_buffer_a.setSize(w, h);
  accumulation_buffer_a.addColorAttachment(ColorFormat::RGBA, ACCUMULATION_COLOR_AND_DEPTH);
  accumulation_buffer_a.addColorAttachment(ColorFormat::RGBA, ACCUMULATION_TEMPORAL_DATA);
  g_buffer.shareDepthStencilAttachment(accumulation_buffer_a);
  accumulation_buffer_a.bindColorAttachments();

  accumulation_buffer_b.init();
  accumulation_buffer_b.setSize(w, h);
  accumulation_buffer_b.addColorAttachment(ColorFormat::RGBA, ACCUMULATION_COLOR_AND_DEPTH);
  accumulation_buffer_b.addColorAttachment(ColorFormat::RGBA, ACCUMULATION_TEMPORAL_DATA);
  g_buffer.shareDepthStencilAttachment(accumulation_buffer_b);
  accumulation_buffer_b.bindColorAttachments();

  directional_shadow_map.init();
  directional_shadow_map.setSize(2048, 2048);
  directional_shadow_map.addColorAttachment(ColorFormat::R, DIRECTIONAL_SHADOW_MAP_CASCADE_1);
  directional_shadow_map.addColorAttachment(ColorFormat::R, DIRECTIONAL_SHADOW_MAP_CASCADE_2);
  directional_shadow_map.addColorAttachment(ColorFormat::R, DIRECTIONAL_SHADOW_MAP_CASCADE_3);
  directional_shadow_map.addColorAttachment(ColorFormat::R, DIRECTIONAL_SHADOW_MAP_CASCADE_4);
  directional_shadow_map.addDepthAttachment();
  directional_shadow_map.bindColorAttachments();

  tachyon->window_width = w;
  tachyon->window_height = h;
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

static void RenderSurface(Tachyon* tachyon, SDL_Surface* surface, int32 x, int32 y, uint32 w, uint32 h, float rotation, const tVec4f& color, const tVec4f& background) {
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
  SetShaderVec4f(locations.offset_and_scale, { offsetX, offsetY, scaleX, scaleY });
  SetShaderFloat(locations.rotation, rotation);
  SetShaderVec4f(locations.color, color);
  SetShaderVec4f(locations.background, background);

  RenderScreenQuad(tachyon);
}

static void RenderText(Tachyon* tachyon, TTF_Font* font, const char* message, int32 x, int32 y, uint32 wrap_width, const tVec4f& color, const tVec4f& background) {
  SDL_Surface* text = TTF_RenderText_Blended_Wrapped(font, message, { 255, 255, 255 }, wrap_width);

  RenderSurface(tachyon, text, x, y, text->w, text->h, 0.f, color, background);

  SDL_FreeSurface(text);
}

static void RenderText(Tachyon* tachyon, TTF_Font* font, const char* message, int32 x, int32 y, uint32 wrap_width, const tVec3f& color, const tVec4f& background) {
  RenderText(tachyon, font, message, x, y, wrap_width, tVec4f(color, 1.f), background);
}

static void HandleDevModeInputs(Tachyon* tachyon) {
  auto& renderer = get_renderer();

  // Toggle the G-Buffer view with TAB
  if (did_press_key(tKey::TAB)) {
    renderer.show_g_buffer_view = !renderer.show_g_buffer_view;
  }

  // Toggle V-Sync with V
  if (did_press_key(tKey::V)) {
    auto swap_interval = SDL_GL_GetSwapInterval();

    std::string message =
      std::string("[Tachyon] V-Sync ") +
      (swap_interval ? "disabled" : "enabled");

    SDL_GL_SetSwapInterval(swap_interval ? 0 : 1);

    add_console_message(message, tVec3f(1.f));
  }
}

static void RenderDebugLabels(Tachyon* tachyon) {
  auto& renderer = get_renderer();
  auto& ctx = renderer.ctx;

  // Developer overlay
  {
    #define String(v) std::to_string(v)

    GLint gpu_available = 0;
    GLint gpu_total = 0;
    const char* vendor = (const char*)glGetString(GL_VENDOR);

    if (strcmp(vendor, "NVIDIA Corporation") == 0) {
      glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &gpu_available);
      glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &gpu_total);
    }

    auto render_fps = uint32(1000000.f / (float)renderer.last_render_time_in_microseconds);
    auto frame_fps = uint32(1000000.f / (float)tachyon->last_frame_time_in_microseconds);
    auto used_gpu_memory = (gpu_total - gpu_available) / 1000;
    auto total_gpu_memory = gpu_total / 1000;

    std::vector<std::string> labels = {
      "View: " + std::string(renderer.show_g_buffer_view ? "G-BUFFER" : "DEFAULT"),
      "Resolution: " + String(ctx.w) + " x " + String(ctx.h),
      "V-Sync: " + std::string(SDL_GL_GetSwapInterval() ? "ON" : "OFF"),
      "GPU Memory: " + String(used_gpu_memory) + "MB / " + String(total_gpu_memory) + "MB",
      "Render time: " + String(renderer.last_render_time_in_microseconds) + "us (" + String(render_fps) + "fps)",
      "Frame time: " + String(tachyon->last_frame_time_in_microseconds) + "us (" + String(frame_fps) + "fps)",
      "Meshes: " + String(renderer.total_meshes_drawn),
      "Triangles: " + String(renderer.total_triangles),
      "  (Cascade 0): " + String(renderer.total_triangles_by_cascade[0]),
      "  (Cascade 1): " + String(renderer.total_triangles_by_cascade[1]),
      "  (Cascade 2): " + String(renderer.total_triangles_by_cascade[2]),
      "  (Cascade 3): " + String(renderer.total_triangles_by_cascade[3]),
      "Vertices: " + String(renderer.total_vertices),
      "  (Cascade 0): " + String(renderer.total_vertices_by_cascade[0]),
      "  (Cascade 1): " + String(renderer.total_vertices_by_cascade[1]),
      "  (Cascade 2): " + String(renderer.total_vertices_by_cascade[2]),
      "  (Cascade 3): " + String(renderer.total_vertices_by_cascade[3]),
      "Point lights: " + String(tachyon->point_lights.size()),
      "Draw calls: " + String(renderer.total_draw_calls),
      "Running time: " + String(tachyon->running_time)
    };

    // Engine labels
    int32 y_offset = 10;

    for (auto& label : labels) {
      RenderText(tachyon, tachyon->developer_overlay_font, label.c_str(), 10, y_offset, ctx.w, tVec3f(1.f), tVec4f(0.f, 0.f, 0.f, 0.6f));

      y_offset += 22;
    }

    y_offset += 25;

    // Custom dev labels
    for (auto& dev_label : tachyon->dev_labels) {
      auto full_label = dev_label.label + ": " + dev_label.message;

      RenderText(tachyon, tachyon->developer_overlay_font, full_label.c_str(), 10, y_offset, ctx.w, tVec3f(1.f), tVec4f(0.2f, 0.2f, 1.f, 0.4f));

      y_offset += 24;
    }
  }

  // Console messages
  {
    auto now = Tachyon_GetMicroseconds();
    auto& console_messages = Tachyon_GetConsoleMessages();
    uint32 y_offset = renderer.ctx.h - console_messages.size() * 30 - 10;

    for (auto& console_message : console_messages) {
      auto& message = console_message.message;
      auto& color = console_message.color;
      auto age = std::min((uint64)20000000, now - console_message.time);
      auto time_left = 20000000 - age;
      auto alpha = std::min(1.f, (float)time_left / 1000000.f);

      RenderText(tachyon, tachyon->developer_overlay_font, message.c_str(), 10, y_offset, ctx.w, tVec4f(color, alpha), tVec4f(0.f));

      y_offset += 30;
    }
  }
}

static void UpdateRendererContext(Tachyon* tachyon) {
  auto& ctx = get_renderer().ctx;
  auto& camera = tachyon->scene.camera;
  tMat4f camera_rotation_matrix = camera.rotation.toMatrix4f();
  int w, h;

  SDL_GL_GetDrawableSize(tachyon->sdl_window, &w, &h);

  ctx.w = w;
  ctx.h = h;

  // @todo make fov/near/far customizable
  ctx.projection_matrix = tMat4f::perspective(camera.fov, 500.f, 10000000.f).transpose();

  ctx.previous_view_matrix = ctx.view_matrix;

  ctx.view_matrix = (
    camera_rotation_matrix *
    tMat4f::translation(camera.position * tVec3f(-1.f))
  ).transpose();

  ctx.view_projection_matrix = (
    (
      camera_rotation_matrix *
      tMat4f::translation(tachyon->scene.transform_origin - camera.position)
    ).transpose() *
    ctx.projection_matrix
  );

  ctx.inverse_projection_matrix = ctx.projection_matrix.inverse();
  ctx.inverse_view_matrix = ctx.view_matrix.inverse();

  ctx.camera_position = camera.position;
}

static inline void SetupDrawElementsIndirectCommand(DrawElementsIndirectCommand& command, const tMeshGeometry& geometry) {
  command.count = geometry.face_element_end - geometry.face_element_start;
  command.firstIndex = geometry.face_element_start;
  command.instanceCount = geometry.instance_count;
  command.baseInstance = geometry.base_instance;
  command.baseVertex = geometry.vertex_start;
}

static void AddDrawElementsIndirectCommands(std::vector<DrawElementsIndirectCommand>& commands, const tMeshRecord& record, uint32& triangle_count, uint32& vertex_count) {
  auto& lod_1 = record.lod_1;
  auto& lod_2 = record.lod_2;
  auto& lod_3 = record.lod_3;

  if (lod_1.instance_count > 0) {
    DrawElementsIndirectCommand command;

    SetupDrawElementsIndirectCommand(command, lod_1);

    commands.push_back(command);

    // @todo dev mode only
    {
      triangle_count += command.count * command.instanceCount;
      vertex_count += (lod_1.vertex_end - lod_1.vertex_start) * command.instanceCount;
    }
  }

  if (lod_2.instance_count > 0) {
    DrawElementsIndirectCommand command;

    SetupDrawElementsIndirectCommand(command, lod_2);

    commands.push_back(command);

    // @todo dev mode only
    {
      triangle_count += command.count * command.instanceCount;
      vertex_count += (lod_2.vertex_end - lod_2.vertex_start) * command.instanceCount;
    }
  }

  if (lod_3.instance_count > 0) {
    DrawElementsIndirectCommand command;

    SetupDrawElementsIndirectCommand(command, lod_3);

    commands.push_back(command);

    // @todo dev mode only
    {
      triangle_count += command.count * command.instanceCount;
      vertex_count += (lod_3.vertex_end - lod_3.vertex_start) * command.instanceCount;
    }
  }
}

static void RenderMeshesByType(Tachyon* tachyon, tMeshType type) {
  auto& renderer = get_renderer();
  auto& gl_mesh_pack = renderer.mesh_pack;

  // Re-buffer any updated instances
  {
    for (auto& record : tachyon->mesh_pack.mesh_records) {
      if (
        record.group.disabled ||
        record.group.total_visible == 0 ||
        record.type != type
      ) {
        continue;
      }

      if (!record.group.buffered) {
        glBindBuffer(GL_ARRAY_BUFFER, gl_mesh_pack.buffers[SURFACE_BUFFER]);
        glBufferSubData(GL_ARRAY_BUFFER, record.group.object_offset * sizeof(uint32), record.group.total_visible * sizeof(uint32), record.group.surfaces);

        glBindBuffer(GL_ARRAY_BUFFER, gl_mesh_pack.buffers[MATRIX_BUFFER]);
        glBufferSubData(GL_ARRAY_BUFFER, record.group.object_offset * sizeof(tMat4f), record.group.total_visible * sizeof(tMat4f), record.group.matrices);
      }

      record.group.buffered = true;
    }
  }

  glBindVertexArray(gl_mesh_pack.vao);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_mesh_pack.ebo);

  // @todo avoid allocating + deleting each frame
  std::vector<DrawElementsIndirectCommand> commands;

  auto& records = tachyon->mesh_pack.mesh_records;

  for (uint32 i = 0; i < records.size(); i++) {
    auto& record = records[i];

    if (
      record.group.disabled ||
      record.group.total_visible == 0 ||
      record.type != type
    ) {
      continue;
    }

    AddDrawElementsIndirectCommands(commands, record, renderer.total_triangles, renderer.total_vertices);

    // @todo dev mode only
    {
      renderer.total_meshes_drawn++;
    }
  }

  glBindBuffer(GL_DRAW_INDIRECT_BUFFER, renderer.indirect_buffer);
  glBufferData(GL_DRAW_INDIRECT_BUFFER, commands.size() * sizeof(DrawElementsIndirectCommand), commands.data(), GL_DYNAMIC_DRAW);

  glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0, commands.size(), 0);

  // @todo dev mode only
  {
    renderer.total_draw_calls += 1;
  }
}

static void RenderPbrMeshes(Tachyon* tachyon) {
  auto& renderer = get_renderer();
  auto& shader = renderer.shaders.main_geometry;
  auto& locations = renderer.shaders.locations.main_geometry;
  auto& ctx = renderer.ctx;

  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);

  renderer.g_buffer.write();

  glClearColor(0.f, 0.f, 0.f, 1.f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  glUseProgram(shader.program);
  SetShaderMat4f(locations.view_projection_matrix, ctx.view_projection_matrix);
  SetShaderVec3f(locations.transform_origin, tachyon->scene.transform_origin);

  RenderMeshesByType(tachyon, PBR_MESH);
}

static void RenderShadowMaps(Tachyon* tachyon) {
  auto& camera = tachyon->scene.camera;
  auto& renderer = get_renderer();
  auto& shader = renderer.shaders.shadow_map;
  auto& locations = renderer.shaders.locations.shadow_map;
  auto& ctx = renderer.ctx;
  auto& gl_mesh_pack = renderer.mesh_pack;

  // @temporary
  // @todo allow multiple directional lights
  auto& directional_light_direction = tachyon->scene.directional_light_direction;

  glBindVertexArray(gl_mesh_pack.vao);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_mesh_pack.ebo);

  renderer.directional_shadow_map.write();

  // Directional shadow map
  for (uint8 attachment = DIRECTIONAL_SHADOW_MAP_CASCADE_1; attachment <= DIRECTIONAL_SHADOW_MAP_CASCADE_4; attachment++) {
    auto cascade_index = attachment - DIRECTIONAL_SHADOW_MAP_CASCADE_1;
    auto light_matrix = CreateCascadedLightMatrix(cascade_index, directional_light_direction, camera);

    renderer.directional_shadow_map.writeToAttachment(cascade_index);

    glClearColor(1.f, 1.f, 1.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shader.program);
    SetShaderMat4f(locations.light_matrix, light_matrix);
    SetShaderVec3f(locations.transform_origin, tachyon->scene.transform_origin);

    // @todo avoid allocating + deleting each frame
    std::vector<DrawElementsIndirectCommand> commands;

    auto& records = tachyon->mesh_pack.mesh_records;

    for (uint32 i = 0; i < records.size(); i++) {
      auto& record = records[i];

      if (
        record.group.disabled ||
        record.group.total_visible == 0 ||
        record.type != PBR_MESH ||
        record.shadow_cascade_ceiling <= cascade_index
      ) {
        continue;
      }

      AddDrawElementsIndirectCommands(commands, record, renderer.total_triangles_by_cascade[cascade_index], renderer.total_vertices_by_cascade[cascade_index]);
    }

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, renderer.indirect_buffer);
    glBufferData(GL_DRAW_INDIRECT_BUFFER, commands.size() * sizeof(DrawElementsIndirectCommand), commands.data(), GL_DYNAMIC_DRAW);

    glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0, commands.size(), 0);

    // @todo dev mode only
    {
      renderer.total_draw_calls += 1;
    }
  }
}

static void RenderGlobalLighting(Tachyon* tachyon) {
  auto& renderer = get_renderer();
  auto& scene = tachyon->scene;
  auto& camera = scene.camera;
  auto& shader = renderer.shaders.global_lighting;
  auto& locations = renderer.shaders.locations.global_lighting;
  auto& ctx = renderer.ctx;

  // @temporary
  // @todo allow multiple directional lights
  auto& directional_light_direction = tachyon->scene.directional_light_direction;

  auto& previous_accumulation_buffer = renderer.current_frame % 2 == 0
    ? renderer.accumulation_buffer_b
    : renderer.accumulation_buffer_a;

  auto& target_accumulation_buffer = renderer.current_frame % 2 == 0
    ? renderer.accumulation_buffer_a
    : renderer.accumulation_buffer_b;

  renderer.g_buffer.read();
  previous_accumulation_buffer.read();
  renderer.directional_shadow_map.read();
  target_accumulation_buffer.write();

  glViewport(0, 0, ctx.w, ctx.h);
  glClear(GL_COLOR_BUFFER_BIT);

  glUseProgram(shader.program);
  SetShaderVec4f(locations.offset_and_scale, { 0.f, 0.f, 1.f, 1.f });
  SetShaderFloat(locations.rotation, 0.f);
  SetShaderInt(locations.in_normal_and_depth, G_BUFFER_NORMALS_AND_DEPTH);
  SetShaderInt(locations.in_color_and_material, G_BUFFER_COLOR_AND_MATERIAL);
  SetShaderInt(locations.in_temporal_data, ACCUMULATION_TEMPORAL_DATA);
  SetShaderInt(locations.in_shadow_map_cascade_1, DIRECTIONAL_SHADOW_MAP_CASCADE_1);
  SetShaderInt(locations.in_shadow_map_cascade_2, DIRECTIONAL_SHADOW_MAP_CASCADE_2);
  SetShaderInt(locations.in_shadow_map_cascade_3, DIRECTIONAL_SHADOW_MAP_CASCADE_3);
  SetShaderInt(locations.in_shadow_map_cascade_4, DIRECTIONAL_SHADOW_MAP_CASCADE_4);
  SetShaderMat4f(locations.light_matrix_cascade_1, CreateCascadedLightMatrix(0, directional_light_direction, camera));
  SetShaderMat4f(locations.light_matrix_cascade_2, CreateCascadedLightMatrix(1, directional_light_direction, camera));
  SetShaderMat4f(locations.light_matrix_cascade_3, CreateCascadedLightMatrix(2, directional_light_direction, camera));
  SetShaderMat4f(locations.light_matrix_cascade_4, CreateCascadedLightMatrix(3, directional_light_direction, camera));
  SetShaderMat4f(locations.projection_matrix, ctx.projection_matrix);
  SetShaderMat4f(locations.view_matrix, ctx.view_matrix);
  SetShaderMat4f(locations.previous_view_matrix, ctx.previous_view_matrix);
  SetShaderMat4f(locations.inverse_projection_matrix, ctx.inverse_projection_matrix);
  SetShaderMat4f(locations.inverse_view_matrix, ctx.inverse_view_matrix);
  SetShaderVec3f(locations.camera_position, ctx.camera_position);
  SetShaderFloat(locations.scene_time, scene.scene_time);
  SetShaderFloat(locations.running_time, tachyon->running_time);
  // @temporary
  // @todo allow multiple directional lights
  SetShaderVec3f(locations.directional_light_direction, scene.directional_light_direction);
  SetShaderBool(locations.use_high_visibility_mode, tachyon->use_high_visibility_mode);

  RenderScreenQuad(tachyon);
}

static void RenderPostMeshes(Tachyon* tachyon) {
  auto& scene = tachyon->scene;
  auto& renderer = get_renderer();
  auto& ctx = renderer.ctx;

  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);

  auto& target_accumulation_buffer = renderer.current_frame % 2 == 0
    ? renderer.accumulation_buffer_a
    : renderer.accumulation_buffer_b;

  // Wireframes
  {
    auto& shader = renderer.shaders.wireframe_mesh;
    auto& locations = renderer.shaders.locations.wireframe_mesh;

    glUseProgram(shader.program);
    SetShaderMat4f(locations.view_projection_matrix, ctx.view_projection_matrix);
    SetShaderVec3f(locations.transform_origin, scene.transform_origin);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glLineWidth(1.5f);

    RenderMeshesByType(tachyon, WIREFRAME_MESH);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }

  // Volumetrics (@todo rename Atmospheres)
  {
    auto& shader = renderer.shaders.volumetric_mesh;
    auto& locations = renderer.shaders.locations.volumetric_mesh;

    glUseProgram(shader.program);
    SetShaderMat4f(locations.view_projection_matrix, ctx.view_projection_matrix);
    SetShaderVec3f(locations.transform_origin, scene.transform_origin);
    SetShaderVec3f(locations.camera_position, ctx.camera_position);
    SetShaderVec3f(locations.primary_light_direction, scene.directional_light_direction);
    SetShaderFloat(locations.scene_time, scene.scene_time);

    RenderMeshesByType(tachyon, VOLUMETRIC_MESH);
  }

  // Fire
  {
    glDepthMask(false);

    auto& shader = renderer.shaders.fire_mesh;
    auto& locations = renderer.shaders.locations.fire_mesh;

    glUseProgram(shader.program);
    SetShaderMat4f(locations.view_projection_matrix, ctx.view_projection_matrix);
    SetShaderVec3f(locations.transform_origin, scene.transform_origin);
    SetShaderVec3f(locations.camera_position, ctx.camera_position);
    SetShaderFloat(locations.scene_time, scene.scene_time);

    RenderMeshesByType(tachyon, FIRE_MESH);

    glDepthMask(true);
  }

  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
}

static void RenderPointLights(Tachyon* tachyon) {
  auto& renderer = get_renderer();
  auto& scene = tachyon->scene;
  auto& camera = scene.camera;
  auto& shader = renderer.shaders.point_lights;
  auto& locations = renderer.shaders.locations.point_lights;
  auto& ctx = renderer.ctx;

  auto& previous_accumulation_buffer = renderer.current_frame % 2 == 0
    ? renderer.accumulation_buffer_b
    : renderer.accumulation_buffer_a;

  auto& target_accumulation_buffer = renderer.current_frame % 2 == 0
    ? renderer.accumulation_buffer_a
    : renderer.accumulation_buffer_b;

  renderer.g_buffer.read();
  previous_accumulation_buffer.read();
  target_accumulation_buffer.write();

  glUseProgram(shader.program);
  SetShaderInt(locations.in_normal_and_depth, G_BUFFER_NORMALS_AND_DEPTH);
  SetShaderInt(locations.in_color_and_material, G_BUFFER_COLOR_AND_MATERIAL);
  SetShaderMat4f(locations.projection_matrix, ctx.projection_matrix);
  SetShaderMat4f(locations.view_matrix, ctx.view_matrix);
  SetShaderMat4f(locations.inverse_projection_matrix, ctx.inverse_projection_matrix);
  SetShaderMat4f(locations.inverse_view_matrix, ctx.inverse_view_matrix);
  SetShaderVec3f(locations.camera_position, ctx.camera_position);

  // @todo perform culling
  auto& lights = tachyon->point_lights;
  auto total_instances = lights.size();
  // @todo avoid reallocating each frame
  auto* instances = new tOpenGLPointLightDiscInstance[total_instances];
  // @todo put in ctx
  auto aspect_ratio = float(ctx.w) / float(ctx.h);

  auto view_matrix = ctx.view_matrix.transpose();
  auto projection_matrix = ctx.projection_matrix.transpose();

  for (uint32 i = 0; i < total_instances; i++) {
    auto& light = lights[i];
    auto& instance = instances[i];

    instance.light = light;

    tVec3f local_light_position = view_matrix * light.position;

    if (light.power == 0.f) {
      instance.offset = tVec2f(0.f);
      instance.scale = tVec2f(0.f);
    } else if (local_light_position.z < -0.1f) {
      // Light source in front of the camera
      tVec3f clip_position = (projection_matrix * local_light_position) / local_light_position.z;

      clip_position.x = 1.f - clip_position.x - 1.f;
      clip_position.y = 1.f - clip_position.y - 1.f;

      instance.offset = tVec2f(clip_position.x, clip_position.y);
      // @todo use 1 + log(light.power) or similar for scaling term
      instance.scale.x = 1.5f * light.radius / local_light_position.z;
      instance.scale.y = 1.5f * light.radius / local_light_position.z * aspect_ratio;
    } else {
      // Light source behind the camera; scale to cover
      // screen when within range, and scale to 0 when
      // out of range
      float scale = local_light_position.magnitude() < light.radius ? 2.f : 0.f;

      instance.offset = tVec2f(0.f);
      instance.scale = tVec2f(scale);
    }
  }

  glBindBuffer(GL_ARRAY_BUFFER, renderer.point_light_disc.light_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(tOpenGLPointLightDiscInstance) * total_instances, instances, GL_DYNAMIC_DRAW);

  glBindVertexArray(renderer.point_light_disc.vao);
  // @todo reference # of disc slices in tachyon_opengl_geometry.cpp
  glDrawArraysInstanced(GL_TRIANGLES, 0, 16 * 3, total_instances);

  delete[] instances;
}

static void RenderPost(Tachyon* tachyon) {
  auto& renderer = get_renderer();
  auto& shader = renderer.shaders.post;
  auto& locations = renderer.shaders.locations.post;
  auto& ctx = renderer.ctx;

  auto& accumulation_buffer = renderer.current_frame % 2 == 0
    ? renderer.accumulation_buffer_a
    : renderer.accumulation_buffer_b;

  accumulation_buffer.read();

  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glViewport(0, 0, ctx.w, ctx.h);

  glUseProgram(shader.program);
  SetShaderVec4f(locations.offset_and_scale, { 0.f, 0.f, 1.f, 1.f });
  SetShaderFloat(locations.rotation, 0.f);
  SetShaderInt(locations.in_color_and_depth, ACCUMULATION_COLOR_AND_DEPTH);

  RenderScreenQuad(tachyon);
}

static void RenderUIElements(Tachyon* tachyon) {
  auto& renderer = get_renderer();
  auto& ctx = renderer.ctx;

  for (auto& command : tachyon->ui_draw_commands) {
    SDL_Surface* surface = nullptr;

    if (command.ui_element != nullptr) {
      surface = command.ui_element->surface;
    } else if (command.ui_text != nullptr) {
      auto& text = *command.ui_text;
      auto& string = command.options.string;

      surface = TTF_RenderText_Blended_Wrapped(text.font, string.c_str(), { 255, 255, 255 }, ctx.w);
    }

    if (surface == nullptr) {
      continue;
    }

    auto half_w = surface->w >> 1;
    auto half_h = surface->h >> 1;
    auto& options = command.options;
    auto x = options.screen_x - half_w;
    auto y = options.screen_y - half_h;

    // @todo batch render common surfaces
    RenderSurface(tachyon, surface, x, y, surface->w, surface->h, options.rotation, tVec4f(options.color, options.alpha), tVec4f(0.f));

    if (command.ui_text != nullptr) {
      SDL_FreeSurface(surface);
    }
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
  SetShaderVec4f(locations.offset_and_scale, { 0.f, 0.f, 1.f, 1.f });
  SetShaderFloat(locations.rotation, 0.f);
  SetShaderInt(locations.in_normal_and_depth, 0);
  SetShaderInt(locations.in_color_and_material, 1);
  SetShaderMat4f(locations.inverse_projection_matrix, ctx.inverse_projection_matrix);
  SetShaderMat4f(locations.inverse_view_matrix, ctx.inverse_view_matrix);

  RenderScreenQuad(tachyon);
}

void Tachyon_OpenGL_InitRenderer(Tachyon* tachyon) {
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

  tachyon->renderer = renderer;

  Tachyon_OpenGL_InitShaders(renderer->shaders);

  // Initialize buffers
  {
    glGenBuffers(1, &renderer->indirect_buffer);
    CreateRenderBuffers(tachyon);
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
    renderer->point_light_disc = Tachyon_CreateOpenGLPointLightDisc(tachyon);
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
}

void Tachyon_OpenGL_ResizeRenderer(Tachyon* tachyon) {
  auto& renderer = get_renderer();

  renderer.g_buffer.destroy();
  renderer.accumulation_buffer_a.destroy();
  renderer.accumulation_buffer_b.destroy();
  renderer.directional_shadow_map.destroy();

  CreateRenderBuffers(tachyon);
}

void Tachyon_OpenGL_RenderScene(Tachyon* tachyon) {
  auto& renderer = get_renderer();
  auto& ctx = renderer.ctx;
  auto start = Tachyon_GetMicroseconds();

  // @todo dev mode only
  // Every second, check for shader changes and do hot reloading
  if (tachyon->running_time - renderer.last_shader_hot_reload_time > 1.f) {
    Tachyon_OpenGL_HotReloadShaders(renderer.shaders);

    renderer.last_shader_hot_reload_time = tachyon->running_time;
  }

  // @todo dev mode only
  // Reset counters
  {
    renderer.total_triangles = 0;
    renderer.total_vertices = 0;
    renderer.total_triangles_by_cascade[0] = 0;
    renderer.total_triangles_by_cascade[1] = 0;
    renderer.total_triangles_by_cascade[2] = 0;
    renderer.total_triangles_by_cascade[3] = 0;
    renderer.total_vertices_by_cascade[0] = 0;
    renderer.total_vertices_by_cascade[1] = 0;
    renderer.total_vertices_by_cascade[2] = 0;
    renderer.total_vertices_by_cascade[3] = 0;
    renderer.total_meshes_drawn = 0;
    renderer.total_draw_calls = 0;
  }

  UpdateRendererContext(tachyon);
  RenderPbrMeshes(tachyon);
  RenderShadowMaps(tachyon);

  // The next steps in the pipeline render quads in screen space,
  // so we don't need to do any back-face culling or depth testing
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);

  if (renderer.show_g_buffer_view) {
    RenderGBufferView(tachyon);
  } else {
    RenderGlobalLighting(tachyon);

    glEnable(GL_BLEND);
    glBlendFuncSeparatei(0, GL_ONE, GL_ONE, GL_ONE, GL_ONE);

    if (!tachyon->use_high_visibility_mode) {
      RenderPointLights(tachyon);
    }

    RenderPostMeshes(tachyon);

    glDisable(GL_BLEND);

    RenderPost(tachyon);
    RenderUIElements(tachyon);
  }

  // @todo dev mode only
  HandleDevModeInputs(tachyon);

  // @todo dev mode only
  if (tachyon->show_developer_tools) {
    RenderDebugLabels(tachyon);
  } else {
    auto frame_fps = uint32(1000000.f / (float)tachyon->last_frame_time_in_microseconds);
    auto label = std::to_string(frame_fps) + "fps";

    RenderText(tachyon, tachyon->developer_overlay_font, label.c_str(), 10, 10, 1920, tVec3f(1.f), tVec4f(0.f));
  }

  SDL_GL_SwapWindow(tachyon->sdl_window);

  renderer.current_frame++;
  renderer.last_render_time_in_microseconds = Tachyon_GetMicroseconds() - start;
}

void Tachyon_OpenGL_DestroyRenderer(Tachyon* tachyon) {
  auto& renderer = get_renderer();

  glDeleteBuffers(1, &renderer.indirect_buffer);
  // @todo DestroyBuffers()

  Tachyon_OpenGL_DestroyShaders(renderer.shaders);

  SDL_GL_DeleteContext(renderer.gl_context);
}