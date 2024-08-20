#pragma once

#include <vector>

#include <SDL.h>
#include <SDL_ttf.h>

#include "engine/tachyon_aliases.h"
#include "engine/tachyon_camera.h"
#include "engine/tachyon_linear_algebra.h"
#include "engine/tachyon_quaternion.h"

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
  tVec3f position;
  tVec3f scale;
  Quaternion rotation;
  tVec4f color;

  uint16 object_index = 0;  // @todo change to object_id, use an id -> index table
  uint16 mesh_index = 0;
};

struct tObjectGroup {
  tObject* objects = nullptr;
  tMat4f* matrices = nullptr;
  tVec4f* colors = nullptr;
  uint32 object_offset = 0;
  uint16 total = 0;
  uint16 total_visible = 0;
  bool buffered = false;

  tObject& operator [](uint16 index) {
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

  uint64 held_key_state = 0;
  uint64 pressed_key_state = 0;
  uint64 released_key_state = 0;

  struct Scene {
    tCamera camera;
    tCamera3p camera3p;
  } scene;

  // @todo dev mode only
  TTF_Font* developer_overlay_font = nullptr;
  uint64 frame_start_time_in_microseconds = 0;
  uint64 last_frame_time_in_microseconds = 1;

  std::vector<tObject> objects;
  std::vector<tMat4f> matrices;
  std::vector<tVec4f> colors; // @todo use a uint32 or other
};