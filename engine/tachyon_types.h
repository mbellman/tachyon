#pragma once

#include <vector>

#include "engine/tachyon_aliases.h"

struct SDL_Window;

enum TachyonRenderBackend {
  OPENGL
};

// @todo move to tachyon_vector.h
struct tVec2f {
  float x = 0.f;
  float y = 0.f;
};

// @todo move to tachyon_vector.h
struct tVec3f {
  float x = 0.f;
  float y = 0.f;
  float z = 0.f;
};

struct tVertex {
  tVec3f position;
  tVec3f normal;
  tVec3f tangent;
  tVec2f uv;
};

struct tMesh {
  std::vector<tVec3f> vertices;
  // @todo material properties
};

struct tMeshRecord {
  uint32 start;
  uint32 end;
};

struct tMeshPack {
  std::vector<tVec3f> vertex_stream;
  std::vector<tMeshRecord> mesh_records;
};

struct Tachyon {
  SDL_Window* sdl_window = nullptr;
  TachyonRenderBackend render_backend = TachyonRenderBackend::OPENGL;
  void* renderer = nullptr;
  bool running = true;

  tMeshPack mesh_pack;
};