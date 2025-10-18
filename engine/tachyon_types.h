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

  tColor(uint16 rgba) : rgba(rgba) {};
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

  bool operator==(const tObject& object) const {
    return object_id == object.object_id && mesh_index == object.mesh_index;
  }
};

struct tObjectGroup {
  tObject* objects = nullptr;
  uint32* surfaces = nullptr;
  tMat4f* matrices = nullptr;
  uint16* id_to_index = nullptr;
  uint32 object_offset = 0;
  uint16 total = 0;
  uint16 total_active = 0;
  uint16 highest_used_id = 0;
  bool buffered = false;
  bool disabled = false;

  std::vector<tObject> initial_objects;

  tObject& operator [](uint16 index) {
    // @todo assert index is in range
    return objects[index];
  }

  tObject* begin() const {
    return objects;
  }

  tObject* end() const {
    return &objects[total_active];
  }

  tObject* getById(uint16 id) const {
    if (id >= total) {
      return nullptr;
    }

    auto index = id_to_index[id];

    if (index > total_active - 1) {
      return nullptr;
    }

    return &objects[index];
  }
};

struct tMesh {
  std::vector<tVertex> vertices;
  std::vector<uint32> face_elements;
};

enum tMeshType {
  PBR_MESH,
  // @todo rename ATMOSPHERE_MESH
  VOLUMETRIC_MESH,
  FIRE_MESH,
  ION_THRUSTER_MESH,
  WIREFRAME_MESH,
  GRASS_MESH
};

struct tMeshGeometry {
  uint32 vertex_start = 0;
  uint32 vertex_end = 0;
  uint32 face_element_start = 0;
  uint32 face_element_end = 0;
  uint32 base_instance = 0;
  uint16 instance_count = 0;
};

struct tMeshRecord {
  tMeshGeometry lod_1;
  tMeshGeometry lod_2;
  tMeshGeometry lod_3;
  uint16 mesh_index;
  uint8 shadow_cascade_ceiling = 4;
  tMeshType type = PBR_MESH;
  std::string texture = "";

  tObjectGroup group;
};

struct tMeshPack {
  std::vector<tVertex> vertex_stream;
  std::vector<uint32> face_element_stream;
  std::vector<tMeshRecord> mesh_records;
};

struct tPointLight {
  int32 id = -1;
  tVec3f position;
  float radius = 1000.f;
  tVec3f color = tVec3f(1.f);
  float power = 1.f;
  float glow_power = 1.f;
};

// @todo dev mode only
struct tDevLabel {
  std::string label;
  std::string message;
};

struct tUIElement {
  SDL_Surface* surface = nullptr;
};

struct tUIText {
  TTF_Font* font = nullptr;
};

struct tUIDrawCommandOptions {
  int32 screen_x = 0;
  int32 screen_y = 0;
  bool centered = true;
  float rotation = 0.f;
  tVec3f color = tVec3f(1.f);
  float alpha = 1.f;
  std::string string = "";
};

struct tUIDrawCommand {
  const tUIElement* ui_element = nullptr;
  const tUIText* ui_text = nullptr;
  tUIDrawCommandOptions options;
};

struct Tachyon {
  SDL_Window* sdl_window = nullptr;
  int32 window_width;
  int32 window_height;
  TachyonRenderBackend render_backend = TachyonRenderBackend::OPENGL;
  void* renderer = nullptr;
  float running_time = 0.f;
  bool is_running = true;
  bool is_window_focused = false;

  tMeshPack mesh_pack;

  // @todo should these go into tMeshPack? use a global array per mesh pack?
  std::vector<tObject> objects;
  std::vector<uint32> surfaces;
  std::vector<tMat4f> matrices;

  std::vector<tPointLight> point_lights;

  std::vector<tUIDrawCommand> ui_draw_commands;

  uint64 held_key_state = 0;
  uint64 pressed_key_state = 0;
  uint64 released_key_state = 0;
  char text_input;
  int32 mouse_delta_x = 0;
  int32 mouse_delta_y = 0;
  tVec2f left_stick;
  tVec2f right_stick;
  float left_trigger = 0.f;
  float right_trigger = 0.f;
  int8 wheel_direction = 0;
  bool did_left_click_down = false;
  bool did_left_click_up = false;
  bool did_right_click_down = false;
  bool did_right_click_up = false;
  bool is_mouse_held_down = false;
  bool is_left_mouse_held_down = false;
  bool is_right_mouse_held_down = false;
  bool is_controller_connected = false;

  struct Scene {
    tCamera camera;
    tCamera3p camera3p;

    tVec3f transform_origin = tVec3f(0.f);
    float scene_time = 0.f;

    float z_near = 500.f;
    float z_far = 10000000.f;

    // @todo allow multiple directional lights
    tVec3f primary_light_direction = tVec3f(0, -1.f, 0);
    tVec3f primary_light_color = tVec3f(1.f);
  } scene;

  struct Fx {
    // Cosmodrone
    float scan_time = 4.f;

    // Alchemist's Astrolabe (?)
    float accumulation_blur_factor = 0.f;
  } fx;

  // @todo dev mode only
  bool show_developer_tools = false;
  bool use_high_visibility_mode = false;
  TTF_Font* developer_overlay_font = nullptr;
  TTF_Font* overlay_message_font = nullptr;
  uint64 frame_start_time_in_microseconds = 0;
  uint64 last_frame_time_in_microseconds = 1;
  std::vector<tDevLabel> dev_labels;
  std::string overlay_message = "";
  float last_overlay_message_time = 0.f;
};