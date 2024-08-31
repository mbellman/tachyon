#pragma once

#include <vector>

#include <SDL_opengl.h>

#include "engine/tachyon_aliases.h"

struct ColorAttachment {
  GLuint index;
  GLuint textureId;
  GLenum textureUnit;
};

enum ColorFormat {
  R,
  R16,
  RG,
  RG16,
  RGB,
  RGB16,
  RGBA,
  RGBA8,
  RGBA8UI,
  RGBA16
};

class OpenGLFrameBuffer {
public:
  void init();
  void destroy();
  void addColorAttachment(ColorFormat format);
  void addColorAttachment(ColorFormat format, uint32 unit);
  void addColorAttachment(ColorFormat format, uint32 unit, GLint clamp);
  void addDepthAttachment();
  void addDepthStencilAttachment();
  void bindColorAttachments();
  void read(uint32 offset = 0);
  void setSize(uint32 width, uint32 height);
  void shareDepthStencilAttachment(const OpenGLFrameBuffer& target);
  void write();
  void writeToAttachment(uint32 attachment);

private:
  GLuint fbo = 0;
  GLuint depthTextureId = 0;
  GLuint depthStencilTextureId = 0;
  std::vector<ColorAttachment> colorAttachments;
  uint32 width, height;
};

class OpenGLCubeMap {
public:
  void init();
  void destroy();
  void read();
  void addColorAttachment(ColorFormat format, uint32 unit);
  void addDepthAttachment(uint32 unit);
  void bindColorAttachments();
  void setSize(uint32 width, uint32 height);
  void write();
  void writeToFace(uint8 face);

private:
  GLuint fbo = 0;
  GLenum depthTextureUnit;
  GLuint depthTextureId = 0;
  std::vector<ColorAttachment> colorAttachments;
  uint32 width, height;
};