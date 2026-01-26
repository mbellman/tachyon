#include "astro/camera_system.h"
#include "astro/entity_manager.h"

using namespace astro;

static void UpdateCameraNearEntities(State& state, const std::vector<GameEntity>& entities, tVec3f& camera_position, const float max_distance, const tVec3f& offset) {
  for (auto& entity : entities) {
    float player_distance = tVec3f::distance(state.player_position, entity.position);

    float distance_factor = 1.f - player_distance / max_distance;
    if (distance_factor < 0.f) distance_factor = 0.f;

    camera_position.y += offset.y * distance_factor;
    camera_position.z += offset.z * distance_factor;
  }
}

static void UpdateCameraNearEntities(State& state, tVec3f& new_camera_position) {
  UpdateCameraNearEntities(state, state.light_posts, new_camera_position, 10000.f, tVec3f(0, 2000.f, 2000.f));
  UpdateCameraNearEntities(state, state.gates, new_camera_position, 10000.f, tVec3f(0, 2000.f, 3000.f));
  UpdateCameraNearEntities(state, state.water_wheels, new_camera_position, 20000.f, tVec3f(0, 3000.f, 5000.f));
  UpdateCameraNearEntities(state, state.lesser_guards, new_camera_position, 20000.f, tVec3f(0, 500.f, 200.f));
  UpdateCameraNearEntities(state, state.low_guards, new_camera_position, 20000.f, tVec3f(0, 500.f, 200.f));
}

static void UpdateCameraDuringEvent(Tachyon* tachyon, State& state, tVec3f& new_camera_position) {
  auto& active_event = state.camera_events[0];
  auto& target_entity = *EntityManager::FindEntity(state, active_event.target_entity_record);

  new_camera_position = target_entity.visible_position;
  new_camera_position.y += 1000.f;
  new_camera_position.z += 2000.f;
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
}

static void UpdateAstroTravelCamera(Tachyon* tachyon, State& state, tVec3f& new_camera_position) {
  // Astro-turning camera; use the player position as with walking/standing still,
  // but apply the camera shift immediately instead of lerping. This ensures that
  // the camera more smoothly blends back into its expected position when a targeted
  // entity ends its lifespan and disappears during an astro turn action. Without
  // this special case, the slower camera shift lerp causes the camera to "curve"
  // as it returns to its expected position, which looks odd.
  new_camera_position = state.player_position;

  UpdateCameraNearEntities(state, new_camera_position);

  tVec3f shift_direction = state.player_facing_direction + tVec3f(0, 0, 0.4f);

  state.camera_shift = shift_direction * tVec3f(0.75f, 0, 1.f) * 1500.f;
}

static void UpdateStandardCamera(Tachyon* tachyon, State& state, tVec3f& new_camera_position) {
  float player_speed = state.player_velocity.magnitude();

  new_camera_position = state.player_position;

  float shift_amount = std::max(player_speed * 1.5f, 1500.f);
  tVec3f shift_direction = state.player_facing_direction + tVec3f(0, 0, 0.4f);
  tVec3f desired_camera_shift = shift_direction * tVec3f(0.75f, 0, 1.f) * shift_amount;

  UpdateCameraNearEntities(state, new_camera_position);

  state.camera_shift = tVec3f::lerp(state.camera_shift, desired_camera_shift, 5.f * state.dt);
}

void CameraSystem::UpdateCamera(Tachyon* tachyon, State& state) {
  profile("UpdateCamera()");

  tVec3f new_camera_position;

  if (state.camera_events.size() > 0) {
    UpdateCameraDuringEvent(tachyon, state, new_camera_position);
  }
  else if (state.has_target) {
    UpdateTargetCamera(tachyon, state, new_camera_position);
  }
  else if (abs(state.astro_turn_speed) > 0.1f) {
    UpdateAstroTravelCamera(tachyon, state, new_camera_position);
  }
  else {
    UpdateStandardCamera(tachyon, state, new_camera_position);
  }

  new_camera_position += state.camera_shift;
  new_camera_position.y += 10000.f;
  new_camera_position.z += 7000.f;

  // @temporary
  // @todo dev mode only
  if (state.use_zoomed_out_camera) {
    new_camera_position.y += 10000.f;
    new_camera_position.z += 10000.f;
  }

  // @temporary
  {
    state.camera_angle = 0.9f;

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

  camera.position = tVec3f::lerp(camera.position, new_camera_position, 5.f * state.dt);
  camera.rotation = Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), state.camera_angle);
}