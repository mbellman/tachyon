#include <glew.h>
#include <SDL.h>
#include <SDL_opengl.h>

#include "engine/tachyon_aliases.h"
#include "engine/tachyon_file_helpers.h"
#include "engine/opengl/tachyon_opengl_renderer.h"

internal GLuint CreateShaderProgram() {
  return glCreateProgram();
}

internal GLuint CreateShader(GLenum type, const char* path) {
  auto shader = glCreateShader(type);
  auto source = Tachyon_GetFileContents(path);
  auto* sourcePointer = source.c_str();

  glShaderSource(shader, 1, &sourcePointer, 0);
  glCompileShader(shader);

  // @todo check status

  return shader;
}

internal void SetupShaders(TachyonOpenGLShaders& shaders) {
  shaders.main_geometry.program = CreateShaderProgram();
  shaders.main_geometry.vertex_shader = CreateShader(GL_VERTEX_SHADER, "./engine/renderers/opengl/main_geometry.vert.glsl");
  shaders.main_geometry.fragment_shader = CreateShader(GL_FRAGMENT_SHADER, "./engine/renderers/opengl/main_geometry.frag.glsl");

  glAttachShader(shaders.main_geometry.program, shaders.main_geometry.vertex_shader);
  glAttachShader(shaders.main_geometry.program, shaders.main_geometry.fragment_shader);
  glLinkProgram(shaders.main_geometry.program);
}

void Tachyon_InitOpenGLRenderer(Tachyon* tachyon) {
  auto* renderer = new TachyonOpenGLRenderer;

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

  tachyon->renderer = renderer;

  SetupShaders(renderer->shaders);
}

void Tachyon_RenderSceneInOpenGL(Tachyon* tachyon) {
  int w, h;

  SDL_GL_GetDrawableSize(tachyon->sdl_window, &w, &h);

  glViewport(0, 0, w, h);
  glClearColor(0.f, 0.f, 0.f, 1.f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);

  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glViewport(0, 0, w, h);

  SDL_GL_SwapWindow(tachyon->sdl_window);
}

void Tachyon_DestroyOpenGLRenderer(Tachyon* tachyon) {
  auto* renderer = (TachyonOpenGLRenderer*)tachyon->renderer;

  SDL_GL_DeleteContext(renderer->context);
}