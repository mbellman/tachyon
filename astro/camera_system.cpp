#include "astro/camera_system.h"
#include "astro/entity_behaviors/behavior.h"
#include "astro/entity_manager.h"
#include "astro/player_character.h"

using namespace astro;

static void UpdateCameraNearInertEntities(State& state, const std::vector<GameEntity>& entities, tVec3f& camera_position, const float max_distance, const tVec3f& offset) {
  for (auto& entity : entities) {
    float player_distance = tVec3f::distance(state.player_position, entity.position);

    float distance_factor = 1.f - player_distance / max_distance;
    if (distance_factor < 0.f) distance_factor = 0.f;

    camera_position.y += offset.y * distance_factor;
    camera_position.z += offset.z * distance_factor;
  }
}

static void UpdateCameraNearLiveEntities(State& state, const std::vector<GameEntity>& entities, tVec3f& camera_position, const float max_distance, const tVec3f& offset) {
  for (auto& entity : entities) {
    if (!IsDuringActiveTime(entity, state)) continue;

    float player_distance = tVec3f::distance(state.player_position, entity.position);

    float distance_factor = 1.f - player_distance / max_distance;
    if (distance_factor < 0.f) distance_factor = 0.f;

    camera_position.y += offset.y * distance_factor;
    camera_position.z += offset.z * distance_factor;
  }
}

static void UpdateCameraNearEntities(State& state, tVec3f& new_camera_position) {
  UpdateCameraNearInertEntities(state, state.light_posts, new_camera_position, 10000.f, tVec3f(0, 2000.f, 2000.f));
  UpdateCameraNearInertEntities(state, state.gates, new_camera_position, 10000.f, tVec3f(0, 2000.f, 3000.f));
  UpdateCameraNearInertEntities(state, state.water_wheels, new_camera_position, 20000.f, tVec3f(0, 3000.f, 5000.f));

  UpdateCameraNearLiveEntities(state, state.lesser_guards, new_camera_position, 20000.f, tVec3f(0, 500.f, 200.f));
  UpdateCameraNearLiveEntities(state, state.low_guards, new_camera_position, 20000.f, tVec3f(0, 500.f, 200.f));
}

static void UpdateEventCamera(Tachyon* tachyon, State& state, tVec3f& new_camera_position) {
  auto& event = state.camera_events[0];
  auto& target_entity = *EntityManager::FindEntity(state, event.target_entity_record);
  // Only apply the camera blend once the event has started,
  // e.g. if the event is intentionally delayed
  float blend_factor = get_scene_time() > event.start_time ? event.blend_factor : 0.f;
  tVec3f base_position = tVec3f::lerp(state.player_position, target_entity.visible_position, blend_factor);

  new_camera_position = base_position;
  new_camera_position.y += 1000.f;
  new_camera_position.z += 2000.f;

  state.camera_blend_speed = 5.f * state.dt;
}

static void UpdateTargetCamera(Tachyon* tachyon, State& state, tVec3f& new_camera_position) {
  auto& target = *EntityManager::FindEntity(state, state.target_entity);
  float player_distance = (state.player_position - target.visible_position).magnitude();
  float time_since_casting_stun = time_since(state.spells.stun_start_time);

  float approach_factor = 1.f - player_distance / 10000.f;
  if (approach_factor < 0.f) approach_factor = 0.f;

  float stun_factor = time_since_casting_stun < t_PI ? sinf(time_since_casting_stun) : 0.f;
  if (stun_factor < 0.f) stun_factor = 0.f;
  if (stun_factor > 1.f) stun_factor = 1.f;

  new_camera_position = tVec3f::lerp(state.player_position, target.visible_position, 0.5f * approach_factor);

  // Adjustment: raise the camera as we approach the target
  // @todo it's confusing that we start at 3000 here and add the final height
  // below, near the end of the procedure
  new_camera_position.y = 3000.f * approach_factor;

  // Adjustment: raise the camera a bit during stun effects
  new_camera_position.y += 1000.f * stun_factor;

  // Adjustment: move the camera back a bit as we approach the target
  new_camera_position.z += 3000.f * approach_factor;

  // Adjustment: move the camera back a bit during stun effects
  new_camera_position.z += 1000.f * stun_factor;

  // Blend between the normal player camera shift and no shift at all
  // depending on how close we are to the target
  tVec3f shift_direction = state.player_facing_direction + tVec3f(0, 0, 0.4f);
  tVec3f desired_camera_shift = shift_direction * tVec3f(0.75f, 0, 1.f) * 1950.f;

  state.camera_shift = tVec3f::lerp(desired_camera_shift, tVec3f(0.f), approach_factor);

  state.camera_blend_speed = 5.f * state.dt;
}

static void UpdatePreviewTargetCamera(Tachyon* tachyon, State& state, tVec3f& new_camera_position) {
  tVec3f average_position = tVec3f(0.f);

  for (auto& record : state.targetable_entities) {
    auto& entity = *EntityManager::FindEntity(state, record);

    average_position += entity.visible_position;
  }

  average_position /= (float)state.targetable_entities.size();

  float player_distance = (state.player_position - average_position).magnitude();

  float approach_factor = 1.f - player_distance / 10000.f;
  if (approach_factor < 0.f) approach_factor = 0.f;

  new_camera_position = tVec3f::lerp(state.player_position, average_position, 0.5f * approach_factor);

  // Adjustment: raise the camera as we approach the target
  // @todo it's confusing that we start at 3000 here and add the final height
  // below, near the end of the procedure
  new_camera_position.y = 3000.f * approach_factor;

  // Adjustment: move the camera back a bit as we approach the target
  new_camera_position.z += 3000.f * approach_factor;

  UpdateCameraNearEntities(state, new_camera_position);

  // Blend between the normal player camera shift and no shift at all
  // depending on how close we are to the target
  tVec3f shift_direction = state.player_facing_direction + tVec3f(0, 0, 0.4f);
  tVec3f desired_camera_shift = shift_direction * tVec3f(0.75f, 0, 1.f) * 1950.f;

  state.camera_shift = tVec3f::lerp(desired_camera_shift, tVec3f(0.f), approach_factor);
  state.camera_blend_speed = Tachyon_Lerpf(state.camera_blend_speed, 2.f * state.dt, state.dt);
}

static void UpdateStandardCamera(Tachyon* tachyon, State& state, tVec3f& new_camera_position) {
  new_camera_position = state.player_position;

  // @todo cleanup/refactor
  float player_speed = state.player_velocity.magnitude();
  float speed_ratio = player_speed / PlayerCharacter::MAX_RUN_SPEED;
  float facing_factor = 1.f + speed_ratio * 0.5f;
  float shift_amount = 1500.f + player_speed * 0.5f;
  tVec3f shift_vector = state.player_facing_direction + tVec3f(0, 0, 0.2f);
  tVec3f desired_camera_shift = shift_vector * tVec3f(0.75f, 0, 1.f) * shift_amount;

  UpdateCameraNearEntities(state, new_camera_position);

  state.camera_shift = tVec3f::lerp(state.camera_shift, desired_camera_shift, 5.f * state.dt);
  state.camera_blend_speed = Tachyon_Lerpf(state.camera_blend_speed, 5.f * state.dt, state.dt);
}

// @todo continue to work on this
static void HandleExperimentalCamera(Tachyon* tachyon, State& state) {
  auto& camera = tachyon->scene.camera;
  float player_angle = atan2f(state.player_facing_direction.x, state.player_facing_direction.z) + t_PI;

  camera.position = state.player_position - state.player_facing_direction * 7500.f;
  camera.position.y += 8000.f;

  camera.rotation = (
    Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), 0.7f) *
    Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), -player_angle)
  );
}

void CameraSystem::UpdateCamera(Tachyon* tachyon, State& state) {
  profile("UpdateCamera()");

  tVec3f new_camera_position;

  if (state.camera_events.size() > 0) {
    UpdateEventCamera(tachyon, state, new_camera_position);
  }
  else if (state.has_target) {
    UpdateTargetCamera(tachyon, state, new_camera_position);
  }
  else if (state.targetable_entities.size() > 0) {
    UpdatePreviewTargetCamera(tachyon, state, new_camera_position);
  }
  else {
    UpdateStandardCamera(tachyon, state, new_camera_position);
  }

  new_camera_position += state.camera_shift;
  new_camera_position.y += 11000.f;
  new_camera_position.z += 9000.f;

  // Limit camera distance to avoid making out-of-view culling obvious
  {
    if (new_camera_position.y - state.player_position.y > 13000.f) {
      new_camera_position.y = state.player_position.y + 13000.f;
    }

    if (new_camera_position.z - state.player_position.z > 12000.f) {
      new_camera_position.z = state.player_position.z + 12000.f;
    }
  }

  // @temporary
  // @todo dev mode only
  if (state.use_zoomed_out_camera) {
    new_camera_position.y += 10000.f;
    new_camera_position.z += 10000.f;
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

  // HandleExperimentalCamera(tachyon, state);
}