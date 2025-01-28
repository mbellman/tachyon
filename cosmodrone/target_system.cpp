#include <algorithm>

#include "cosmodrone/autopilot.h"
#include "cosmodrone/target_system.h"

using namespace Cosmodrone;

const static float MAX_TARGET_DISTANCE = 200000.f;

static bool IsTrackingObject(State& state, const tObject& object) {
  for (auto& tracker : state.on_screen_target_trackers) {
    if (tracker.object == object) {
      return true;
    }
  }

  return false;
}

static void StartTrackingObject(State& state, const tObject& object) {
  state.on_screen_target_trackers.push_back({
    .object = object,
    .activated_time = state.current_game_time,
    .selected_time = 0.f,
    .deselected_time = 0.f,
    .deactivated_time = 0.f
  });
}

static void StopTrackingObject(State& state, const tObject& object) {
  auto& trackers = state.on_screen_target_trackers;

  for (uint32 i = 0; i < trackers.size(); i++) {
    if (trackers[i].object == object) {
      trackers.erase(trackers.begin() + i);

      return;
    }
  }

  // @todo
  // for (auto& tracker : state.on_screen_target_trackers) {
  //   if (tracker.object == object) {
  //     tracker.deactivated_time = state.current_game_time;

  //     return;
  //   }
  // }
}

void TargetSystem::HandleTargetTrackers(Tachyon* tachyon, State& state, const float dt) {
  auto& camera = tachyon->scene.camera;

  // Manage tracker instances
  {
    static std::vector<uint16> standard_docking_target_meshes = {
      state.meshes.antenna_3,
      state.meshes.charge_pad,
      state.meshes.fighter,
      state.meshes.floater_1
    };

    for (auto mesh_index : standard_docking_target_meshes) {
      for (auto& object : objects(mesh_index)) {
        auto camera_to_object = object.position - camera.position;
        auto object_direction = camera_to_object.unit();
        auto target_dot = tVec3f::dot(object_direction, state.view_forward_direction);

        if (
          camera_to_object.magnitude() > MAX_TARGET_DISTANCE ||
          (state.is_piloting_vehicle && object == state.docking_target) ||
          target_dot < 0.8f
        ) {
          if (
            IsTrackingObject(state, object) &&
            // Perform a special check to avoid un-tracking docking targets
            // when they go offscreen while auto-docking, provided they are
            // still technically in front of the camera.
            !(
              state.flight_mode == FlightMode::AUTO_DOCK &&
              object == state.docking_target &&
              target_dot > 0.f
            )
          ) {
            StopTrackingObject(state, object);
          }

          continue;
        }

        if (!IsTrackingObject(state, object)) {
          StartTrackingObject(state, object);
        }
      }
    }

    for (auto& object : objects(state.meshes.zone_target)) {
      auto camera_to_object = object.position - camera.position;
      auto object_direction = camera_to_object.unit();
      auto target_dot = tVec3f::dot(object_direction, state.view_forward_direction);

      if (
        camera_to_object.magnitude() > 5000000.f ||
        target_dot < 0.8f
      ) {
        if (IsTrackingObject(state, object)) {
          StopTrackingObject(state, object);
        }

        continue;
      }

      if (!IsTrackingObject(state, object)) {
        StartTrackingObject(state, object);
      }
    }
  }

  // Recalculate/redraw
  {
    auto center_x = int32(tachyon->window_width >> 1);
    auto center_y = int32(tachyon->window_height >> 1);
    float closest_distance_to_center = std::numeric_limits<float>::max();
    float closest_relative_distance = std::numeric_limits<float>::max();
    tObject selected_target;

    tMat4f view_matrix = (
      camera.rotation.toMatrix4f() *
      tMat4f::translation(camera.position * tVec3f(-1.f))
    );

    // @todo make fov/near/far customizable
    tMat4f projection_matrix = tMat4f::perspective(camera.fov, 500.f, 10000000.f);

    // Calculate tracker screen coordinates
    for (auto& tracker : state.on_screen_target_trackers) {
      // @todo use live object
      auto& object = tracker.object;
      tVec3f local_position = view_matrix * object.position;
      tVec3f clip_position = (projection_matrix * local_position) / local_position.z;

      clip_position.x *= 0.5f;
      clip_position.x += 0.5f;
      clip_position.y *= 0.5f;
      clip_position.y += 0.5f;

      auto screen_x = int32(tachyon->window_width - clip_position.x * tachyon->window_width);
      auto screen_y = int32(clip_position.y * tachyon->window_height);

      if (screen_x < 0) screen_x = 0;
      if (screen_x > tachyon->window_width) screen_x = tachyon->window_width;

      if (screen_y < 0) screen_y = 0;
      if (screen_y > tachyon->window_height) screen_y = tachyon->window_height;

      tracker.screen_x = screen_x;
      tracker.screen_y = screen_y;

      float dx = float(screen_x - center_x);
      float dy = float(screen_y - center_y);

      float center_distance = sqrtf(dx*dx + dy*dy);
      float relative_distance = (object.position - state.ship_position).magnitude();

      tracker.center_distance = center_distance;
      tracker.object_distance = relative_distance;

      // Try to select the target tracker closest to the center of the screen,
      // assuming its object is closer than the closest relative distance threshold
      if (
        relative_distance < closest_relative_distance &&
        center_distance < closest_distance_to_center &&
        // @temporary
        // @todo come up with a different scheme for targeting zone targets
        tracker.object.mesh_index != state.meshes.zone_target
      ) {
        closest_distance_to_center = center_distance;
        selected_target = tracker.object;
      }

      // Prioritize closer target objects
      if (
        relative_distance < 20000.f &&
        relative_distance < closest_relative_distance &&
        // @temporary
        // @todo come up with a different scheme for targeting zone targets
        tracker.object.mesh_index != state.meshes.zone_target
      ) {
        closest_relative_distance = relative_distance;
        selected_target = tracker.object;
      }
    }

    // In auto-dock mode, permanently keep the docking target selected
    if (state.flight_mode == FlightMode::AUTO_DOCK) {
      selected_target = state.docking_target;
    }

    // Update tracker selection (only when not auto-docking)
    for (auto& tracker : state.on_screen_target_trackers) {
      if (
        tracker.object != selected_target &&
        tracker.selected_time != 0.f
      ) {
        // Deselect any previously-selected trackers
        // if they are not for the selected target object
        tracker.selected_time = 0.f;
        tracker.deselected_time = state.current_game_time;
      }

      if (
        tracker.object == selected_target &&
        tracker.selected_time == 0.f &&
        tracker.object.mesh_index != state.meshes.zone_target
      ) {
        // Select the tracker for the selected target object
        tracker.selected_time = state.current_game_time;
        tracker.deselected_time = 0.f;
      }
    }

    // @todo remove this once we move tracker drawing into TargetSystem
    if (Autopilot::IsDoingDockingApproach(state)) {
      return;
    }

    // @experimental
    if (state.photo_mode) {
      return;
    }

    // Draw trackers
    // @todo move to hud_system.cpp
    for (auto& tracker : state.on_screen_target_trackers) {
      if (tracker.selected_time != 0.f) {
        // Draw selected tracker
        const static float animation_time = 0.3f;

        Tachyon_DrawUIElement(tachyon, state.ui.selected_target_center, {
          .screen_x = tracker.screen_x,
          .screen_y = tracker.screen_y
        });

        auto time_since_selected = state.current_game_time - tracker.selected_time;
        if (time_since_selected > animation_time) time_since_selected = animation_time;

        const static std::vector<tVec2f> offsets = {
          tVec2f(-1.f, -1.f),
          tVec2f(1.f, -1.f),
          tVec2f(1.f, 1.f),
          tVec2f(-1.f, 1.f)
        };

        const static std::vector<float> rotations = {
          0.f,
          t_PI * 0.5f,
          t_PI,
          t_PI + t_HALF_PI
        };

        auto alpha = time_since_selected / animation_time;
        alpha = sqrtf(alpha);

        auto spread = 30.f + 10.f * (1.f - alpha);

        for (uint32 i = 0; i < 4; i++) {
          auto& offset = offsets[i];
          auto rotation = rotations[i];
          int32 screen_x = tracker.screen_x + int32(offset.x * spread);
          int32 screen_y = tracker.screen_y + int32(offset.y * spread);

          Tachyon_DrawUIElement(tachyon, state.ui.selected_target_corner, {
            .screen_x = screen_x,
            .screen_y = screen_y,
            .rotation = rotation
          });
        }

        // Draw focus indicator
        Tachyon_DrawUIElement(tachyon, state.ui.target_focus, {
          .screen_x = tracker.screen_x,
          .screen_y = tracker.screen_y,
          .rotation = 2.f * state.current_game_time,
          .alpha = (0.8f + 0.2f * sinf(state.current_game_time * t_TAU))
        });
      } else {
        // Draw unselected trackers
        int32 minimum_edge_distance = std::min({
          tracker.screen_x,
          (int32)tachyon->window_width - tracker.screen_x,
          tracker.screen_y,
          (int32)tachyon->window_height - tracker.screen_y
        });

        float edge_distance_factor = (float)minimum_edge_distance / 200.f;
        float world_distance_factor = 1.f - (tracker.object.position - camera.position).magnitude() / MAX_TARGET_DISTANCE;
        float alpha = edge_distance_factor * world_distance_factor;
        if (alpha < 0.f) alpha = 0.f;
        if (alpha > 1.f) alpha = 1.f;

        if (tracker.object.mesh_index == state.meshes.zone_target) {
          Tachyon_DrawUIElement(tachyon, state.ui.zone_target_indicator, {
            .screen_x = tracker.screen_x,
            .screen_y = tracker.screen_y,
            .alpha = 1.f
          });
        } else {
          Tachyon_DrawUIElement(tachyon, state.ui.target_indicator, {
            .screen_x = tracker.screen_x,
            .screen_y = tracker.screen_y,
            .alpha = alpha
          });
        }
      }
    }
  }
}

void TargetSystem::UpdateTargetStats(Tachyon* tachyon, State& state) {
  auto* target = TargetSystem::GetSelectedTargetTracker(state);

  if (target == nullptr) {
    return;
  }

  auto& stats = state.target_stats;
  auto* live_object = get_original_object(target->object);
  float target_distance = (live_object->position - state.ship_position).magnitude();

  stats.distance_in_meters = uint32(target_distance / 1000.f);
  stats.unit_direction = (live_object->position - state.ship_position).unit();
  // @todo compute proper relative velocity (as well as + or - depending on
  // whether we're approaching/receding from the target)
  stats.relative_velocity = state.ship_velocity.magnitude() / 1000.f;
}

const TargetTracker* TargetSystem::GetSelectedTargetTracker(State& state) {
  for (auto& tracker : state.on_screen_target_trackers) {
    if (tracker.selected_time != 0.f) {
      return &tracker;
    }
  }

  return nullptr;
}