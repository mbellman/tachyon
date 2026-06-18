#include "astro/camera_system.h"
#include "astro/entity_behaviors/behavior.h"
#include "astro/entity_manager.h"
#include "astro/player_character.h"

using namespace astro;

static void UpdateCameraNearInertEntities(State& state, const std::vector<GameEntity>& entities, const float max_distance, const tVec3f& offset) {
  for (auto& entity : entities) {
    float player_distance = tVec3f::distance(state.player_position, entity.position);

    float distance_factor = 1.f - player_distance / max_distance;
    if (distance_factor < 0.f) distance_factor = 0.f;

    state.camera_tracking_position.y += offset.y * distance_factor;
    state.camera_tracking_position.z += offset.z * distance_factor;
  }
}

static void UpdateCameraNearLiveEntities(State& state, const std::vector<GameEntity>& entities, const float max_distance, const tVec3f& offset) {
  for (auto& entity : entities) {
    if (!IsDuringActiveTime(entity, state)) continue;

    float player_distance = tVec3f::distance(state.player_position, entity.position);

    float distance_factor = 1.f - player_distance / max_distance;
    if (distance_factor < 0.f) distance_factor = 0.f;

    state.camera_tracking_position.y += offset.y * distance_factor;
    state.camera_tracking_position.z += offset.z * distance_factor;
  }
}

static void UpdateCameraNearEntities(State& state) {
  UpdateCameraNearInertEntities(state, state.light_posts, 10000.f, tVec3f(0, 2000.f, 2000.f));
  UpdateCameraNearInertEntities(state, state.gates, 10000.f, tVec3f(0, 2000.f, 3000.f));
  UpdateCameraNearInertEntities(state, state.water_wheels, 20000.f, tVec3f(0, 3000.f, 5000.f));

  if (!state.enemies_disabled) {
    UpdateCameraNearLiveEntities(state, state.lesser_guards, 20000.f, tVec3f(0, 1000.f, 200.f));
    UpdateCameraNearLiveEntities(state, state.low_guards, 20000.f, tVec3f(0, 1000.f, 200.f));
  }
}

static void UpdateEventCamera(State& state, const float scene_time) {
  auto& event = state.camera_events[0];
  auto& target_entity = *EntityManager::FindEntity(state, event.target_entity_record);
  // Only apply the camera blend once the event has started,
  // e.g. if the event is intentionally delayed
  float blend_factor = scene_time > event.start_time ? event.blend_factor : 0.f;
  tVec3f base_position = tVec3f::lerp(state.player_position, target_entity.visible_position, blend_factor);

  state.camera_tracking_position = base_position;
  state.camera_blend_speed = 5.f * state.dt;
}

static void UpdateLadderCamera(State& state) {
  state.camera_tracking_position = state.player_position;

  state.camera_blend_speed = Tachyon_Lerpf(
    state.camera_blend_speed,
    state.dt,
    state.dt
  );
}

static void UpdateStandardCamera(State& state) {
  state.camera_tracking_position = state.player_position;

  state.camera_blend_speed = Tachyon_Lerpf(
    state.camera_blend_speed,
    10.f * state.dt,
    state.dt
  );
}

void CameraSystem::UpdateCamera(Tachyon* tachyon, State& state) {
  profile("UpdateCamera()");

  if (state.camera_events.size() > 0) {
    UpdateEventCamera(state, get_scene_time());
  } else if (state.is_on_ladder || PlayerCharacter::IsClimbingOffLadder(tachyon, state)) {
    UpdateLadderCamera(state);
  } else {
    UpdateStandardCamera(state);
  }

  UpdateCameraNearEntities(state);

  state.camera_offset_position = tVec3f::lerp(
    state.camera_offset_position,
    state.player_facing_direction * 1500.f,
    state.dt
  );

  tVec3f new_camera_position = state.camera_tracking_position;
  new_camera_position += state.camera_offset_position;
  new_camera_position.y += 11000.f;
  new_camera_position.z += 8500.f;

  state.camera_height_adjustment = Tachyon_Lerpf(
    state.camera_height_adjustment,
    state.target_camera_height_adjustment,
    0.5f * state.dt
  );

  new_camera_position.y += state.camera_height_adjustment;
  new_camera_position.z += state.camera_height_adjustment;

  // @temporary
  // @todo dev mode only
  if (state.use_zoomed_out_camera) {
    new_camera_position.y += 15000.f;
    new_camera_position.z += 15000.f;
  }

  // Vantage camera
  {
    float transition_duration = state.use_vantage_camera ? 2.f : 1.f;

    float alpha = time_since(state.vantage_camera_change_time) / transition_duration;
    if (alpha < 0.f) alpha = 0.f;
    if (alpha > 1.f) alpha = 1.f;
    alpha = Tachyon_EaseInOutf(alpha);

    if (state.use_vantage_camera) {
      state.camera_angle = Tachyon_Lerpf(state.recorded_camera_angle, 0.2f, alpha);

      new_camera_position.y -= 3000.f * alpha;
      new_camera_position.z += 7000.f * alpha;
    } else {
      state.camera_angle = Tachyon_Lerpf(state.recorded_camera_angle, 0.9f, alpha);

      new_camera_position.y -= 4000.f * (1.f - alpha);
      new_camera_position.z += 5000.f * (1.f - alpha);
    }
  }

  // @temporary
  // @todo formalize special camera modes
  {
    for (auto& entity : state.npcs) {
      if (entity.unique_name == "lake_girl") {
        // if (abs(entity.position.y - state.player_position.y) > 250.f) continue;

        float distance = tVec3f::distance(state.player_position, entity.position);

        float alpha = Tachyon_InverseLerp(8000.f, 15000.f, distance);
        alpha = Tachyon_EaseInOutf(1.f - alpha);

        state.camera_angle = Tachyon_Lerpf(state.camera_angle, 0.7f, alpha);

        new_camera_position.z += 5000.f * alpha;
      }
    }
  }

  auto& camera = tachyon->scene.camera;
  camera.position = tVec3f::lerp(camera.position, new_camera_position, state.camera_blend_speed);
  camera.rotation = Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), state.camera_angle);

  // camera.rotation = camera.rotation * Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), 0.2f * sinf(get_scene_time()));
}