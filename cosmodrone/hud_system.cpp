#include <format>

#include "cosmodrone/autopilot.h"
#include "cosmodrone/beacons.h"
#include "cosmodrone/hud_system.h"
#include "cosmodrone/target_system.h"

using namespace Cosmodrone;

static void HandleDroneInspector(Tachyon* tachyon, State& state, const float dt) {
  auto& meshes = state.meshes;
  auto& camera = tachyon->scene.camera;
  // @todo define an orthonormal view basis and precalculate this
  auto left = tVec3f::cross(state.view_forward_direction, state.view_up_direction).invert();

  auto offset_position =
    camera.position +
    state.view_forward_direction * 550.f +
    left * (0.f + camera.fov * 7.5f);

  // Drone wireframe
  {
    if (objects(meshes.drone_wireframe).total_active == 0) {
      create(meshes.drone_wireframe);
    }

    auto& wireframe = objects(meshes.drone_wireframe)[0];

    wireframe.scale = 1.5f * camera.fov;
    wireframe.color = tVec4f(0.2f, 0.5f, 1.f, 1.f);

    wireframe.position =
      offset_position +
      state.view_forward_direction * 20.f -
      state.view_up_direction * (20.f + 3.f * camera.fov);

    wireframe.rotation =
      camera.rotation.opposite() *
      Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), 0.04f * sinf(state.current_game_time * 0.25f)) *
      Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), state.current_game_time * 0.5f);

    // @experimental
    if (state.photo_mode) {
      wireframe.scale = 0.f;
    }

    commit(wireframe);
  }

  // @experimental
  if (state.photo_mode) {
    return;
  }

  // Drone details
  {
    int32 x = int32(tachyon->window_width * 0.04f);
    int32 y = int32(tachyon->window_height * 0.92f);

    Tachyon_DrawUIText(tachyon, state.ui.cascadia_mono_26, {
      .screen_x = x,
      .screen_y = y,
      .centered = false,
      .color = tVec3f(0.1f, 1.f, 0.7f),
      .string = "C.DRONE-IV 7a."
    });

    Tachyon_DrawUIText(tachyon, state.ui.cascadia_mono_20, {
      .screen_x = x,
      .screen_y = y + 34,
      .centered = false,
      .color = tVec3f(0.1f, 1.f, 0.7f),
      .alpha = 0.8f + 0.2f * sinf(state.current_game_time * 2.f),
      .string = "RANK. [D];"
    });
  }
}

// @todo move to engine?
struct Vec2i {
  int32 x = 0;
  int32 y = 0;
};

static Vec2i GetFlightReticleCoordinates(Tachyon* tachyon, const State& state) {
  auto& offset = state.flight_reticle_offset;

  auto screen_x = (int32)roundf(tachyon->window_width / 2.f + offset.x * 500.f);
  auto screen_y = (int32)roundf(tachyon->window_height / 2.f + offset.y * 500.f);

  return Vec2i(screen_x, screen_y);
}

static void HandleFlightReticle(Tachyon* tachyon, State& state, const float dt) {
  auto& target_offset = state.flight_target_reticle_offset;
  auto& offset = state.flight_reticle_offset;
  auto& rotation = state.flight_reticle_rotation;

  // Lag when panning the camera/rotating
  {
    if (is_window_focused()) {
      target_offset.x -= (float)tachyon->mouse_delta_x / 2000.f;
      target_offset.y -= (float)tachyon->mouse_delta_y / 2000.f;
    }

    rotation -= state.camera_roll_speed * dt;
  }

  // Draw the reticle
  {
    auto coords = GetFlightReticleCoordinates(tachyon, state);

    auto alpha = 1.f - sqrtf(
      offset.x * offset.x +
      offset.y * offset.y
    );

    if (alpha < 0.5f) alpha = 0.5f;

    Tachyon_DrawUIElement(tachyon, state.ui.reticle, {
      .screen_x = coords.x,
      .screen_y = coords.y,
      .rotation = rotation,
      .alpha = alpha
    });
  }

  // Drift toward the center of the screen/stop roll
  {
    target_offset.x *= 1.f - 2.f * dt;
    target_offset.y *= 1.f - 2.f * dt;
    rotation *= 1.f - 4.f * dt;

    offset.x = Tachyon_Lerpf(offset.x, target_offset.x, 20.f * dt);
    offset.y = Tachyon_Lerpf(offset.y, target_offset.y, 20.f * dt);
  }
}

static void HandleTargetLine(Tachyon* tachyon, State& state, const float dt) {
  const static float STEP_SIZE = 50.f;

  auto* tracker = TargetSystem::GetSelectedTargetTracker(state);

  if (tracker == nullptr) {
    return;
  }

  Vec2i start = GetFlightReticleCoordinates(tachyon, state);
  Vec2i end = Vec2i(tracker->screen_x, tracker->screen_y);

  float dx = float(end.x - start.x);
  float dy = float(end.y - start.y);
  float screen_distance = sqrtf(dx*dx + dy*dy);
  uint8 total_dots = uint8(screen_distance / STEP_SIZE);
  tVec2f direction = tVec2f(dx / screen_distance, dy / screen_distance);

  // Make the arrows move toward the target
  float int_part;
  float offset_alpha = modf(state.current_game_time * 2.f, &int_part);

  Vec2i offset = {
    int32(direction.x * offset_alpha * STEP_SIZE),
    int32(direction.y * offset_alpha * STEP_SIZE)
  };

  for (uint8 i = 1; i < total_dots; i++) {
    float float_i = float(i);
    auto screen_x = offset.x + start.x + int32(direction.x * STEP_SIZE * float_i);
    auto screen_y = offset.y + start.y + int32(direction.y * STEP_SIZE * float_i);
    float rotation = atan2f(direction.y, direction.x) + t_HALF_PI;
    float index_ratio = float_i / float(total_dots);
    float alpha = sinf(index_ratio * t_PI);

    Tachyon_DrawUIElement(tachyon, state.ui.dot, {
      .screen_x = screen_x,
      .screen_y = screen_y,
      .rotation = rotation,
      .alpha = alpha
    });
  }

  // Draw details under the selected target.
  // @todo this should be done in a HandleSelectedTargetIndicator() function
  {
    // Show the distance indicator on the top or bottom of the target indicator
    // depending on whether the indicator is clipped on the bottom screen edge
    int32 screen_y =
      tracker->screen_y > tachyon->window_height - 80
        ? tracker->screen_y - 65
        : tracker->screen_y + 60;

    Tachyon_DrawUIText(tachyon, state.ui.cascadia_mono_26, {
      .screen_x = tracker->screen_x,
      .screen_y = screen_y,
      .string = std::to_string(state.target_stats.distance_in_meters) + "m"
    });
  }
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

static float GetWireframeAlpha(const float age) {
  auto clamped = std::clamp(age * 1.2f, 0.f, 1.f);

  if (
    clamped < 0.1f ||
    (clamped > 0.2f && clamped < 0.3f)
  ) {
    return 0.f;
  }

  return std::min(clamped * 3.f, 1.f);
}

static void HandleTargetInspectorWireframe(Tachyon* tachyon, const State& state, const tVec3f& offset_position, const TargetTracker& tracker) {
  auto& camera = tachyon->scene.camera;
  auto time_since_selected = state.current_game_time - tracker.selected_time;
  auto wireframe_mesh_index = GetTargetInspectorWireframeMeshIndex(tracker.object.mesh_index, state);
  auto& objects = objects(wireframe_mesh_index);
  // @todo define an orthonormal view basis and precalculate this
  auto left = tVec3f::cross(state.view_forward_direction, state.view_up_direction).invert();

  if (objects.total_active == 0) {
    create(wireframe_mesh_index);
  }

  auto& wireframe = objects[0];

  wireframe.scale = 50.f;

  wireframe.color = tVec4f(
    tVec3f(0.5f, 0.8f, 1.f),
    GetWireframeAlpha(time_since_selected - 0.2f)
  );

  wireframe.position =
    offset_position +
    state.view_forward_direction * 20.f +
    state.view_up_direction * (80.f + camera.fov);

  wireframe.rotation =
    camera.rotation.opposite() *
    Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), 0.5f * sinf(state.current_game_time * 0.5f)) *
    Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), state.current_game_time);

  // @experimental
  if (state.photo_mode) {
    wireframe.scale = 0.f;
  }

  commit(wireframe);
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

static float GetTextAlpha(const float text_age) {
  auto clamped = std::clamp(text_age * 1.2f, 0.f, 1.f);

  if (clamped < 0.05f || (clamped > 0.1f && clamped < 0.15f)) {
    return 0.f;
  }

  return std::min(clamped * 3.f, 1.f);
}

static std::string GetCondensedFloatString(const float value) {
  auto formatted = std::format("{:.2f}", value);

  if (formatted[0] == '0') {
    formatted.erase(0, 1);
  }

  if (formatted[0] == '-' && formatted[1] == '0') {
    formatted.erase(1, 1);
  }

  return formatted;
}

static std::string GetTargetName(const State& state, const tObject& target) {
  auto& meshes = state.meshes;

  if (target.mesh_index == meshes.antenna_3) {
    return "ANTENNA_3";
  }

  if (target.mesh_index == meshes.fighter) {
    return "FIGHTER";
  }

  return "--UNNAMED--";
}

static void HandleTargetInspectorStats(Tachyon* tachyon, const State& state, const TargetTracker& tracker) {
  auto time_since_selected = state.current_game_time - tracker.selected_time;
  auto target = tracker.object;
  auto wireframe_mesh_index = GetTargetInspectorWireframeMeshIndex(tracker.object.mesh_index, state);
  auto& wireframe = objects(wireframe_mesh_index)[0];
  auto rotation = wireframe.rotation * target.rotation;

  int32 x = int32(tachyon->window_width * 0.86f);
  int32 y = int32(tachyon->window_height * 0.38f);

  // Display target object name
  {
    float int_part;
    auto name_color_blend_factor = 1.f - modf(state.current_game_time * 0.6f, &int_part);
    auto name_color = tVec3f::lerp(tVec3f(0.3f, 0.7f, 1.f), tVec3f(0.9f, 1.f, 1.f), name_color_blend_factor);

    Tachyon_DrawUIText(tachyon, state.ui.cascadia_mono_26, {
      .screen_x = x,
      .screen_y = y,
      .centered = false,
      .color = name_color,
      .alpha = 0.75f + 0.25f * name_color_blend_factor,
      .string = GetTargetName(state, target)
    });
  }

  // Display rotation hex values (purely stylistic)
  auto rx = QuaternionFloatToHex(rotation.x);
  auto ry = QuaternionFloatToHex(rotation.y);
  auto rz = QuaternionFloatToHex(rotation.z);
  auto rw = QuaternionFloatToHex(rotation.w);

  {
    Tachyon_DrawUIText(tachyon, state.ui.cascadia_mono_32, {
      .screen_x = x,
      .screen_y = y + 30,
      .centered = false,
      .color = tVec3f(0.7f, 0.1f, 1.f),
      .alpha = GetTextAlpha(time_since_selected),
      .string = rx + " " + ry + " " + rz + " " + rw
    });
  }

  // Highlight each rotation hex value (purely stylistic)
  {
    const static std::vector<int8> cycle_indexes = { 3, 0, 2, 1, 1, 3, 2, 0, 1, 2, 0, 3, 1, 2 };
    int8 cycle_index = cycle_indexes[int32(state.current_game_time * 3.f) % cycle_indexes.size()];
    int32 highlight_x = x + cycle_index * 57;

    auto& highlight_text =
      cycle_index == 0 ? rx :
      cycle_index == 1 ? ry :
      cycle_index == 2 ? rz :
      cycle_index == 3 ? rw : rx;

    Tachyon_DrawUIText(tachyon, state.ui.cascadia_mono_32, {
      .screen_x = highlight_x,
      .screen_y = y + 30,
      .centered = false,
      .color = tVec3f(0.7f, 7.f, 1.f),
      .alpha = GetTextAlpha(time_since_selected - 0.1f),
      .string = highlight_text
    });
  }

  // Display additional stats/details
  {
    auto& stats = state.target_stats;

    {
      auto distance_string =
        "DIST(m). +" +
        std::to_string(stats.distance_in_meters) +
        ";";

      Tachyon_DrawUIText(tachyon, state.ui.cascadia_mono_20, {
        .screen_x = x,
        .screen_y = y + 70,
        .centered = false,
        .color = tVec3f(0.1f, 1.f, 0.7f),
        .alpha = GetTextAlpha(time_since_selected - 0.2f),
        .string = distance_string
      });
    }

    {
      auto direction_string =
        "DIR(N). " +
        GetCondensedFloatString(stats.unit_direction.x) +
        GetCondensedFloatString(stats.unit_direction.y) +
        GetCondensedFloatString(stats.unit_direction.z) +
        ";";

      Tachyon_DrawUIText(tachyon, state.ui.cascadia_mono_20, {
        .screen_x = x,
        .screen_y = y + 100,
        .centered = false,
        .color = tVec3f(0.1f, 1.f, 0.7f),
        .alpha = GetTextAlpha(time_since_selected - 0.3f),
        .string = direction_string
      });
    }

    {
      auto screen_coordinates_string =
        "S.COORD. " +
        std::to_string(tracker.screen_x) + " " +
        std::to_string(tracker.screen_y) +
        ";";

      Tachyon_DrawUIText(tachyon, state.ui.cascadia_mono_20, {
        .screen_x = x,
        .screen_y = y + 130,
        .centered = false,
        .color = tVec3f(0.1f, 1.f, 0.7f),
        .alpha = GetTextAlpha(time_since_selected - 0.4f),
        .string = screen_coordinates_string
      });
    }

    {
      auto relative_velocity_string =
        "R-VEL. " +
        GetCondensedFloatString(stats.relative_velocity) +
        "m/s\x00b2;";

      Tachyon_DrawUIText(tachyon, state.ui.cascadia_mono_20, {
        .screen_x = x,
        .screen_y = y + 160,
        .centered = false,
        .color = tVec3f(0.1f, 1.f, 0.7f),
        .alpha = GetTextAlpha(time_since_selected - 0.5f),
        .string = relative_velocity_string
      });
    }
  }
}

static void HandleTargetInspector(Tachyon* tachyon, State& state, const float dt) {
  auto& meshes = state.meshes;
  auto& camera = tachyon->scene.camera;
  auto left = tVec3f::cross(state.view_forward_direction, state.view_up_direction).invert();

  auto offset_position =
    camera.position +
    state.view_forward_direction * 550.f -
    left * camera.fov * 7.75f;

  auto* tracker = TargetSystem::GetSelectedTargetTracker(state);

  if (tracker != nullptr) {
    HandleTargetInspectorWireframe(tachyon, state, offset_position, *tracker);

    // @experimental
    if (!state.photo_mode) {
      HandleTargetInspectorStats(tachyon, state, *tracker);
    }
  } else {
    ResetTargetInspectorObjects(tachyon, state);
  }
}

static void HandleFlightMeter(Tachyon* tachyon, State& state) {
  Tachyon_DrawUIElement(tachyon, state.ui.flight_meter, {
    .screen_x = 180,
    .screen_y = int32(tachyon->window_height / 2 - 25),
    .alpha = 0.5f
  });
}

static void HandlePlaneMeter(Tachyon* tachyon, State& state) {
  Tachyon_DrawUIElement(tachyon, state.ui.plane_meter, {
    .screen_x = int32(tachyon->window_width / 2),
    .screen_y = 80,
    .alpha = 0.25f
  });
}

void HUDSystem::HandleHUD(Tachyon* tachyon, State& state, const float dt) {
  Beacons::UpdateBeacons(tachyon, state);
 
  HandleDroneInspector(tachyon, state, dt);
  HandleTargetInspector(tachyon, state, dt);

  // @experimental
  if (state.photo_mode) {
    return;
  }

  HandleFlightMeter(tachyon, state);
  HandlePlaneMeter(tachyon, state);

  if (!Autopilot::IsDoingDockingApproach(state)) {
    HandleFlightReticle(tachyon, state, dt);
    HandleTargetLine(tachyon, state, dt);
  }
}