#pragma once

#include <string>
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

struct tColor {
  uint16 rgba;

  tColor(const tVec3f& color) : tColor(tVec4f(color.x, color.y, color.z, 0)) {};

  tColor(const tVec4f& color) {
    uint16 r = uint16(color.x * 15.f) << 12;
    uint16 g = uint16(color.y * 15.f) << 8;
    uint16 b = uint16(color.z * 15.f) << 4;
    uint16 a = uint16(color.w * 15.f);

    rgba = r | g | b | a;
  }
};

struct tMaterial {
  uint16 data;

  tMaterial(const tVec4f& material) {
    uint16 roughness = uint16(material.x * 15.f) << 12;
    uint16 metalness = uint16(material.y * 15.f) << 8;
    uint16 clearcoat = uint16(material.z * 15.f) << 4;
    uint16 subsurface = uint16(material.w * 15.f);

    data = roughness | metalness | clearcoat | subsurface;
  }
};

struct tObject {
  tVec3f position;
  tVec3f scale;
  Quaternion rotation = Quaternion(1.f, 0, 0, 0);
  tColor color = tVec3f(1.f);
  tMaterial material = tVec4f(0.6f, 0, 0, 0);

  uint16 object_id = 0;
  uint16 mesh_index = 0;
};

struct tObjectGroup {
  tObject* objects = nullptr;
  uint32* surfaces = nullptr;
  tMat4f* matrices = nullptr;
  uint16* id_to_index = nullptr;
  uint32 object_offset = 0;
  uint16 total = 0;
  uint16 total_visible = 0;
  uint16 total_active = 0;
  bool buffered = false;
  bool disabled = false;

  tObject& operator [](uint16 index) {
    return objects[index];
  }

  tObject* begin() const {
    return objects;
  }

  tObject* end() const {
    return &objects[total_active];
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

struct tDevLabel {
  std::string label;
  std::string message;
};

struct Tachyon {
  SDL_Window* sdl_window = nullptr;
  TachyonRenderBackend render_backend = TachyonRenderBackend::OPENGL;
  void* renderer = nullptr;
  float running_time = 0.f;
  bool is_running = true;
  bool is_window_focused = false;

  tMeshPack mesh_pack;

  uint64 held_key_state = 0;
  uint64 pressed_key_state = 0;
  uint64 released_key_state = 0;
  int32 mouse_delta_x = 0;
  int32 mouse_delta_y = 0;
  int8 wheel_direction = 0;
  bool did_left_click_down = false;
  bool did_left_click_up = false;
  bool did_right_click_down = false;
  bool did_right_click_up = false;
  bool is_mouse_held_down = false;

  std::vector<tObject> objects;
  std::vector<uint32> surfaces;
  std::vector<tMat4f> matrices;

  struct Scene {
    tCamera camera;
    tCamera3p camera3p;

    // @temporary
    // @todo allow multiple directional lights
    tVec3f directional_light_direction = tVec3f(0, -1.f, 0);
  } scene;

  // @todo dev mode only
  bool show_developer_tools = true;
  TTF_Font* developer_overlay_font = nullptr;
  uint64 frame_start_time_in_microseconds = 0;
  uint64 last_frame_time_in_microseconds = 1;
  std::vector<tDevLabel> dev_labels;
};