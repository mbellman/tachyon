#include "astro/camera_system.h"

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

  auto room_camera_position = GetRoomCameraPosition(tachyon, state);

  tVec3f distance_from_room_center;
  distance_from_room_center.x = state.player_position.x - room_camera_position.x;
  distance_from_room_center.z = state.player_position.z - room_camera_position.z;

  // @temporary
  tVec3f new_camera_position;
  new_camera_position.x = room_camera_position.x + distance_from_room_center.x * 0.1f;
  new_camera_position.y = 10000.f;
  new_camera_position.z = 10000.f + room_camera_position.z + distance_from_room_center.z * 0.1f;

  // @temporary
  int32 room_x = (int32)roundf(state.player_position.x / 16000.f);
  int32 room_z = (int32)roundf(state.player_position.z / 16000.f);

  // @todo refactor
  if (room_x != 0 || room_z != 0) {
    float player_speed = state.player_velocity.magnitude();

    if (player_speed > 0.01f) {
      tVec3f unit_velocity = state.player_velocity / player_speed;
      tVec3f camera_bias = tVec3f(0, 0, 0.25f) * abs(unit_velocity.z);
      tVec3f new_camera_shift = (unit_velocity + camera_bias) * 2000.f;

      state.camera_shift = tVec3f::lerp(state.camera_shift, new_camera_shift, 2.f * dt);
    }

    new_camera_position = state.player_position + state.camera_shift;
    new_camera_position.y += 10000.f;
    new_camera_position.z += 7000.f;
  }

  camera.position = tVec3f::lerp(camera.position, new_camera_position, 5.f * dt);
  camera.rotation = Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), 0.9f);
}