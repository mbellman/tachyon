#include "astro/camera_system.h"
#include "astro/entity_manager.h"

using namespace astro;

static tVec3f GetRoomCameraPosition(Tachyon* tachyon, State& state) {
  // @temporary
  float room_size = 16000.f;

  // @temporary
  int32 room_x = (int32)roundf(state.player_position.x / room_size);
  int32 room_z = (int32)roundf(state.player_position.z / room_size);

  // @todo find the room we're inside by examining different room bounds,
  // and determine its center point
  return tVec3f(
    float(room_x) * 16000.f,
    10000.f,
    float(room_z) * 16000.f
  );
}

void CameraSystem::UpdateCamera(Tachyon* tachyon, State& state, const float dt) {
  auto& camera = tachyon->scene.camera;
  tVec3f new_camera_position;

  // @temporary
  int32 room_x = (int32)roundf(state.player_position.x / 16000.f);
  int32 room_z = (int32)roundf(state.player_position.z / 16000.f);

  // @todo refactor
  if (room_x != 0 || room_z != 0) {
    // World camera
    float player_speed = state.player_velocity.magnitude();

    if (state.has_target) {
      // Target camera
      auto& target = *EntityManager::FindEntity(state, state.target_entity);
      float distance = (state.player_position - target.visible_position).magnitude();
      float time_since_casting_stun = tachyon->running_time - state.spells.stun_start_time;

      float distance_ratio = 1.f - distance / 10000.f;
      if (distance_ratio < 0.f) distance_ratio = 0.f;

      float time_ratio = (tachyon->running_time - state.target_start_time) / 1.f;
      if (time_ratio > 1.f) time_ratio = 1.f;

      float stun_factor = time_since_casting_stun < t_PI ? sinf(time_since_casting_stun) : 0.f;
      if (stun_factor < 0.f) stun_factor = 0.f;
      if (stun_factor > 1.f) stun_factor = 1.f;

      new_camera_position = tVec3f::lerp(state.player_position, target.visible_position, 0.5f * distance_ratio);
      new_camera_position += state.player_facing_direction * 2000.f * sqrtf(distance_ratio);
      new_camera_position.y = 3000.f * distance_ratio + 1000.f * stun_factor;
      new_camera_position.z += abs(state.player_facing_direction.z) * 3000.f * sqrtf(distance_ratio);

      state.camera_shift = tVec3f::lerp(state.camera_shift, tVec3f(0.f), time_ratio);
    }
    else if (abs(state.astro_turn_speed) > 0.1f) {
      // Astro-turning camera; use the player position as with walking/standing still,
      // but apply the camera shift immediately instead of lerping. This ensures that
      // the camera more smoothly blends back into its expected position when a targeted
      // entity ends its lifespan and disappears during an astro turn action. Without
      // this special case, the slower camera shift lerp causes the camera to "curve"
      // as it returns to its expected position, which looks odd. 
      new_camera_position = state.player_position;

      tVec3f shift_direction = state.player_facing_direction + tVec3f(0, 0, 0.25f);

      state.camera_shift = shift_direction * tVec3f(0.5f, 0, 1.f) * 800.f;
    }
    else {
      // Walking/standing still camera
      new_camera_position = state.player_position;

      float shift_amount = std::max(player_speed * 2.f, 800.f);
      tVec3f shift_direction = state.player_facing_direction + tVec3f(0, 0, 0.25f);
      tVec3f desired_camera_shift = shift_direction * tVec3f(0.5f, 0, 1.f) * shift_amount;

      state.camera_shift = tVec3f::lerp(state.camera_shift, desired_camera_shift, dt);
    }

    new_camera_position += state.camera_shift;
    new_camera_position.y += 10000.f;
    new_camera_position.z += 7000.f;
  } else {
    // Room camera
    // @todo come up with a better system for this
    auto room_camera_position = GetRoomCameraPosition(tachyon, state);

    tVec3f distance_from_room_center;
    distance_from_room_center.x = state.player_position.x - room_camera_position.x;
    distance_from_room_center.z = state.player_position.z - room_camera_position.z;

    new_camera_position.x = room_camera_position.x + distance_from_room_center.x * 0.1f;
    new_camera_position.y = 10000.f;
    new_camera_position.z = 10000.f + room_camera_position.z + distance_from_room_center.z * 0.1f;
  }

  camera.position = tVec3f::lerp(camera.position, new_camera_position, 5.f * dt);
  camera.rotation = Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), 0.9f);
}