#include <format>

#include "cosmodrone/hud_system.h"
#include "cosmodrone/target_system.h"

using namespace Cosmodrone;

// @todo move to engine
inline float Lerpf(float a, float b, float alpha) {
  return a + (b - a) * alpha;
}

// @todo move to engine
inline tVec3f Lerpf(const tVec3f& a, const tVec3f& b, const float alpha) {
  return tVec3f(
    Lerpf(a.x, b.x, alpha),
    Lerpf(a.y, b.y, alpha),
    Lerpf(a.z, b.z, alpha)
  );
}

static std::string QuaternionFloatToHex(const float value) {
  float normalized = value * 0.5f + 0.5f;
  uint8 value8 = uint8(normalized * 255.f);
  auto formatted = std::format("{:X}", value8);

  if (formatted.size() == 1) {
    formatted = "0" + formatted;
  }

  return formatted;
}

static void ResetTargetInspectorObjects(Tachyon* tachyon, State& state) {
  #define remove_objects(mesh_index)\
    if (objects(mesh_index).total_active > 0) {\
      remove(objects(mesh_index)[0]);\
    }\

  auto& meshes = state.meshes;

  remove_objects(meshes.antenna_3_wireframe);
}

static uint16 GetTargetInspectorWireframeMeshIndex(const uint16 source_mesh, const State& state) {
  auto& meshes = state.meshes;

  if (source_mesh == meshes.antenna_3) {
    return meshes.antenna_3_wireframe;
  }

  return meshes.antenna_3_wireframe;
}

static void HandleTargetInspectorWireframe(Tachyon* tachyon, const State& state, const tVec3f& offset, const TargetTracker& tracker) {
  auto& camera = tachyon->scene.camera;
  auto wireframe_mesh_index = GetTargetInspectorWireframeMeshIndex(tracker.object.mesh_index, state);
  auto& objects = objects(wireframe_mesh_index);
  // @todo define an orthonormal view basis and precalculate this
  auto left = tVec3f::cross(state.view_forward_direction, state.view_up_direction).invert();

  if (objects.total_active == 0) {
    create(wireframe_mesh_index);
  }

  auto& wireframe = objects[0];

  wireframe.scale = 50.f;
  wireframe.color = tVec4f(0.2f, 0.5f, 1.f, 1.f);

  wireframe.position =
    offset +
    state.view_forward_direction * 20.f +
    left * 40.f +
    state.view_up_direction * 110.f;

  wireframe.rotation =
    camera.rotation.opposite() *
    Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), 0.5f * sinf(state.current_game_time * 0.5f)) *
    Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), state.current_game_time);

  commit(wireframe);
}

static void HandleTargetInspectorStats(Tachyon* tachyon, const State& state, const TargetTracker& tracker) {
  auto tracker_object = tracker.object;
  auto wireframe_mesh_index = GetTargetInspectorWireframeMeshIndex(tracker.object.mesh_index, state);
  auto& wireframe = objects(wireframe_mesh_index)[0];
  auto rotation = wireframe.rotation * tracker_object.rotation;

  auto rx = QuaternionFloatToHex(rotation.x);
  auto ry = QuaternionFloatToHex(rotation.y);
  auto rz = QuaternionFloatToHex(rotation.z);
  auto rw = QuaternionFloatToHex(rotation.w);

  int32 x = int32(tachyon->window_width * 0.85f);
  int32 y = int32(tachyon->window_height * 0.4f);

  // Target object name
  {
    float int_part;
    auto name_alpha = 1.f - modf(state.current_game_time * 0.6f, &int_part);
    auto name_color = Lerpf(tVec3f(0.3f, 0.7f, 1.f), tVec3f(0.9f, 1.f, 1.f), name_alpha);

    Tachyon_DrawUIText(tachyon, state.ui.cascadia_mono_26, {
      .screen_x = x,
      .screen_y = y,
      .color = name_color,
      // @temporary
      // @todo determine proper name for target object
      .string = "ANTENNA_3"
    });
  }

  // Rotation hex values (purely stylistic)
  {
    Tachyon_DrawUIText(tachyon, state.ui.cascadia_mono_32, {
      .screen_x = x,
      .screen_y = y + 30,
      .color = tVec3f(0.7f, 0.4f, 1.f),
      .string = rx + " " + ry + " " + rz + " " + rw
    });
  }

  // Highlight each rotation hex value (purely stylistic)
  {
    const static std::vector<int8> cycle_indexes = { 3, 0, 2, 1, 1, 3, 2, 0, 1, 2, 0, 3, 1, 2 };
    int8 cycle_index = cycle_indexes[int32(state.current_game_time * 3.f) % cycle_indexes.size()];
    int32 highlight_x = x - 85 + cycle_index * 57;

    auto& highlight_text =
      cycle_index == 0 ? rx :
      cycle_index == 1 ? ry :
      cycle_index == 2 ? rz :
      cycle_index == 3 ? rw : rx;

    Tachyon_DrawUIText(tachyon, state.ui.cascadia_mono_32, {
      .screen_x = highlight_x,
      .screen_y = y + 30,
      .color = tVec3f(0.7f, 7.f, 1.f),
      .string = highlight_text
    });
  }
}

static void HandleOdometer(Tachyon* tachyon, State& state, const float dt) {
  auto& meshes = state.meshes;
  auto& camera = tachyon->scene.camera;
  auto& wedge = objects(meshes.hud_wedge)[0];
  // @todo define an orthonormal view basis and precalculate this
  auto left = tVec3f::cross(state.view_forward_direction, state.view_up_direction).invert();

  wedge.position = camera.position + state.view_forward_direction * 550.f + left * (200.f + camera.fov * 3.f);
  wedge.scale = 150.f + camera.fov * 1.8f;
  wedge.color = tVec4f(0.1f, 0.2f, 1.f, 1.f);

  wedge.rotation =
    camera.rotation.opposite() *
    Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), t_PI * 1.3f);

  commit(wedge);
}

static void HandleTargetInspector(Tachyon* tachyon, State& state, const float dt) {
  auto& meshes = state.meshes;
  auto& camera = tachyon->scene.camera;
  auto& wedge = objects(meshes.hud_wedge)[1];
  auto left = tVec3f::cross(state.view_forward_direction, state.view_up_direction).invert();

  wedge.position =
    camera.position +
    state.view_forward_direction * 550.f -
    left * (200.f + camera.fov * 3.f);

  wedge.scale = 150.f + camera.fov * 1.8f;
  wedge.color = tVec4f(0.1f, 0.2f, 1.f, 1.f);

  wedge.rotation =
    camera.rotation.opposite() *
    Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), -t_PI * 1.3f) *
    Quaternion::fromAxisAngle(tVec3f(0, 0, 1.f), t_PI);

  commit(wedge);

  auto* tracker = TargetSystem::GetSelectedTargetTracker(state);

  if (tracker != nullptr) {
    HandleTargetInspectorWireframe(tachyon, state, wedge.position, *tracker);
    HandleTargetInspectorStats(tachyon, state, *tracker);
  } else {
    ResetTargetInspectorObjects(tachyon, state);
  }
}

void HUDSystem::HandleHUD(Tachyon* tachyon, State& state, const float dt) {
  HandleOdometer(tachyon, state, dt);
  HandleTargetInspector(tachyon, state, dt);
}