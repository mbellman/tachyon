#pragma once

#include <vector>

#include "engine/tachyon_aliases.h"
#include "engine/tachyon_linear_algebra.h"
#include "engine/tachyon_quaternion.h"

struct SDL_Window;

enum TachyonRenderBackend {
  OPENGL
};

struct tVertex {
  tVec3f position;
  tVec3f normal;
  tVec3f tangent;
  tVec2f uv;
};

struct tObject {
  uint32 index = 0;
  tVec3f position;
  tVec3f scale;
  Quaternion rotation;
  tVec4f color;
};

struct tObjectGroup {
  tObject* objects = nullptr;
  uint32 total = 0;

  tObject& operator [](uint32 index) {
    return objects[index];
  }

  tObject* begin() const {
    return objects;
  }

  tObject* end() const {
    return objects + total;
  }
};

struct tMesh {
  std::vector<tVertex> vertices;
  std::vector<uint32> face_elements;
  // @todo material properties
};

struct tMeshRecord {
  uint32 vertex_start;
  uint32 vertex_end;
  uint32 face_element_start;
  uint32 face_element_end;

  tObjectGroup group;
};

struct tMeshPack {
  std::vector<tVertex> vertex_stream;
  std::vector<uint32> face_element_stream;
  std::vector<tMeshRecord> mesh_records;
};

struct Tachyon {
  SDL_Window* sdl_window = nullptr;
  TachyonRenderBackend render_backend = TachyonRenderBackend::OPENGL;
  void* renderer = nullptr;
  bool running = true;
  float running_time = 0.f;

  tMeshPack mesh_pack;

  std::vector<tObject> objects;
  std::vector<tMat4f> matrices;
  std::vector<tVec4f> colors; // @todo use a uint32 or other
};