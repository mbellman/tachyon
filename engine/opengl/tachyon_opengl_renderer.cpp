#include <glew.h>
#include <SDL.h>
#include <SDL_opengl.h>

#include "engine/tachyon_aliases.h"
#include "engine/tachyon_file_helpers.h"
#include "engine/opengl/tachyon_opengl_renderer.h"

// --------------------------------------
// @todo move to tachyon_opengl_shaders.h
internal GLuint Tachyon_CreateShaderProgram() {
  return glCreateProgram();
}

internal GLuint Tachyon_CreateShader(GLenum type, const char* path) {
  auto shader = glCreateShader(type);
  auto source = Tachyon_GetFileContents(path);
  auto* sourcePointer = source.c_str();

  glShaderSource(shader, 1, &sourcePointer, 0);
  glCompileShader(shader);

  // @todo check status

  return shader;
}

internal void Tachyon_SetupShaders(tOpenGLShaders& shaders) {
  shaders.main_geometry.program = Tachyon_CreateShaderProgram();
  shaders.main_geometry.vertex_shader = Tachyon_CreateShader(GL_VERTEX_SHADER, "./engine/renderers/opengl/main_geometry.vert.glsl");
  shaders.main_geometry.fragment_shader = Tachyon_CreateShader(GL_FRAGMENT_SHADER, "./engine/renderers/opengl/main_geometry.frag.glsl");

  glAttachShader(shaders.main_geometry.program, shaders.main_geometry.vertex_shader);
  glAttachShader(shaders.main_geometry.program, shaders.main_geometry.fragment_shader);
  glLinkProgram(shaders.main_geometry.program);
}

internal void Tachyon_UseShader(tOpenGLShader& shader) {
  glUseProgram(shader.program);
}
// --------------------------------------

// ---------------------------------------
// @todo move to tachyon_opengl_geometry.h
#define VERTEX_BUFFER 0
#define COLOR_BUFFER 1
#define MATRIX_BUFFER 2

#define VERTEX_POSITION 0
#define VERTEX_NORMAL 1
#define VERTEX_TANGENT 2
#define VERTEX_UV 3
#define MODEL_COLOR 4
#define MODEL_MATRIX 5

// @todo verify that this works
internal tOpenGLMeshPack Tachyon_CreateOpenGLMeshPack(Tachyon* tachyon) {
  auto pack = tachyon->mesh_pack;
  auto vertices = pack.vertex_stream;
  auto faceElements = pack.face_element_stream;
  tOpenGLMeshPack glPack;

  glGenVertexArrays(1, &glPack.vao);
  glGenBuffers(3, &glPack.buffers[0]);
  glGenBuffers(1, &glPack.ebo);

  glBindVertexArray(glPack.vao);

  // Buffer vertex data
  glBindBuffer(GL_ARRAY_BUFFER, glPack.buffers[VERTEX_BUFFER]);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(tVertex), vertices.data(), GL_STATIC_DRAW);
  
  // Buffer vertex element data
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glPack.ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, faceElements.size() * sizeof(uint32), faceElements.data(), GL_STATIC_DRAW);

  // Define vertex attributes
  glBindBuffer(GL_ARRAY_BUFFER, glPack.buffers[VERTEX_BUFFER]);

  glEnableVertexAttribArray(VERTEX_POSITION);
  glVertexAttribPointer(VERTEX_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(tVertex), (void*)offsetof(tVertex, position));

  glEnableVertexAttribArray(VERTEX_NORMAL);
  glVertexAttribPointer(VERTEX_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(tVertex), (void*)offsetof(tVertex, normal));

  glEnableVertexAttribArray(VERTEX_TANGENT);
  glVertexAttribPointer(VERTEX_TANGENT, 3, GL_FLOAT, GL_FALSE, sizeof(tVertex), (void*)offsetof(tVertex, tangent));

  glEnableVertexAttribArray(VERTEX_UV);
  glVertexAttribPointer(VERTEX_UV, 2, GL_FLOAT, GL_FALSE, sizeof(tVertex), (void*)offsetof(tVertex, uv));

  // Define color attributes
  glBindBuffer(GL_ARRAY_BUFFER, glPack.buffers[COLOR_BUFFER]);
  glEnableVertexAttribArray(MODEL_COLOR);
  glVertexAttribIPointer(MODEL_COLOR, 1, GL_UNSIGNED_INT, sizeof(tVec4f), (void*)0);
  glVertexAttribDivisor(MODEL_COLOR, 1);

  // Define matrix attributes
  glBindBuffer(GL_ARRAY_BUFFER, glPack.buffers[MATRIX_BUFFER]);

  for (uint32 i = 0; i < 4; i++) {
    glEnableVertexAttribArray(MODEL_MATRIX + i);
    glVertexAttribPointer(MODEL_MATRIX + i, 4, GL_FLOAT, GL_FALSE, sizeof(tMat4f), (void*)(i * 4 * sizeof(float)));
    glVertexAttribDivisor(MODEL_MATRIX + i, 1);
  }

  return glPack;
}

internal void Tachyon_RenderOpenGLMeshPack(Tachyon* tachyon, const tOpenGLMeshPack& glPack) {
  // @todo
}
// ---------------------------------------

void Tachyon_InitOpenGLRenderer(Tachyon* tachyon) {
  auto* renderer = new tOpenGLRenderer;

  // @todo only do GL setup stuff once in case we destroy/recreate the renderer
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

  renderer->context = SDL_GL_CreateContext(tachyon->sdl_window);

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
  }

  tachyon->renderer = renderer;
}

void Tachyon_RenderSceneInOpenGL(Tachyon* tachyon) {
  tOpenGLRenderer& renderer = *(tOpenGLRenderer*)tachyon->renderer;
  int w, h;

  SDL_GL_GetDrawableSize(tachyon->sdl_window, &w, &h);

  glViewport(0, 0, w, h);
  glClearColor(0.f, 0.f, 0.f, 1.f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);

  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glViewport(0, 0, w, h);

  // @todo move to its own function
  {
    // auto& glPack = renderer.mesh_pack;

    // Tachyon_UseShader(renderer.shaders.main_geometry);

    // glBindVertexArray(glPack.vao);
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glPack.ebo);

    // struct DrawElementsIndirectCommand {
    //   GLuint count;
    //   GLuint instanceCount;
    //   GLuint firstIndex;
    //   GLuint baseVertex;
    //   GLuint baseInstance;
    // };

    // auto& records = tachyon->mesh_pack.mesh_records;
    // auto totalMeshes = records.size();
    // auto* commands = new DrawElementsIndirectCommand[totalMeshes];

    // for (uint32 i = 0; i < totalMeshes; i++) {
    //   auto& command = commands[i];
    //   auto& record = records[i];

    //   command.count = record.face_element_end - record.face_element_start;
    //   command.firstIndex = record.face_element_start;
    //   command.instanceCount = 1;
    //   command.baseInstance = 0; // @todo
    //   command.baseVertex = 0;   // @todo
    // }

    // glBindBuffer(GL_DRAW_INDIRECT_BUFFER, renderer.indirect_buffer);
    // glBufferData(GL_DRAW_INDIRECT_BUFFER, totalMeshes * sizeof(DrawElementsIndirectCommand), commands, GL_DYNAMIC_DRAW);

    // delete[] commands;
  }

  SDL_GL_SwapWindow(tachyon->sdl_window);
}

void Tachyon_DestroyOpenGLRenderer(Tachyon* tachyon) {
  auto* renderer = (tOpenGLRenderer*)tachyon->renderer;

  glDeleteBuffers(1, &renderer->indirect_buffer);
  // @todo destroy shaders/buffers/etc.

  SDL_GL_DeleteContext(renderer->context);
}