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
  float delta_factor = 5.f;
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
      auto* entity = EntityManager::FindEntity(state, state.target_entity);
      float distance = (state.player_position - entity->position).magnitude();
      float ratio = 1.f - distance / 14000.f;
      if (ratio < 0.f) ratio = 0.f;

      new_camera_position = tVec3f::lerp(state.player_position, entity->position, ratio);
      delta_factor = Tachyon_Lerpf(5.f, 2.f, sqrtf(ratio));

      state.camera_shift = tVec3f::lerp(state.camera_shift, tVec3f(0, 0, 1000.f), 2.f * dt);
    }
    else if (player_speed > 0.01f) {
      // Walking camera
      tVec3f unit_velocity = state.player_velocity / player_speed;
      tVec3f camera_bias = tVec3f(0, 0, 0.25f) * abs(unit_velocity.z);
      tVec3f new_camera_shift = (unit_velocity + camera_bias) * 2000.f;

      new_camera_position = state.player_position;

      state.camera_shift = tVec3f::lerp(state.camera_shift, new_camera_shift, 2.f * dt);
    }
    else {
      // Standing still camera
      new_camera_position = state.player_position;
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

  camera.position = tVec3f::lerp(camera.position, new_camera_position, delta_factor * dt);
  camera.rotation = Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), 0.9f);
}