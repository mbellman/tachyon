#include "cosmodrone/vehicles.h"

using namespace Cosmodrone;

// @todo move to constants
const static auto FORWARD_VECTOR = tVec3f(0, 0, -1.f);
const static auto UP_VECTOR = tVec3f(0, 1.f, 0);
const static auto RIGHT_VECTOR = tVec3f(1.f, 0, 0);

// @todo remove in favor of LookRotation()
static Quaternion DirectionToQuaternion(const tVec3f& direction) {
  auto yaw = atan2f(direction.x, direction.z);
  auto pitch = atan2f(direction.xz().magnitude(), direction.y) - t_HALF_PI;

  return (
    Quaternion::fromAxisAngle(UP_VECTOR, yaw) *
    Quaternion::fromAxisAngle(RIGHT_VECTOR, pitch)
  );
}

void Vehicles::LoadVehicleMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  meshes.npc_drone_1 = Tachyon_AddMesh(
    tachyon,
    Tachyon_LoadMesh("./cosmodrone/assets/npc-ships/npc_drone_1.obj"),
    200
  );
}

void Vehicles::InitVehicles(Tachyon* tachyon, State& state) {
  auto& point_lights = tachyon->point_lights;
  auto& vehicles = state.vehicles;
  auto& meshes = state.meshes;

  // Reset vehicle instances/objects
  {
    vehicles.clear();

    remove_all(meshes.npc_drone_1);
  }

  for (uint8 i = 0; i < 75; i++) {
    tVec3f direction = tVec3f(
      Tachyon_GetRandom(-1.f, 1.f),
      Tachyon_GetRandom(-1.f, 1.f),
      Tachyon_GetRandom(-1.f, 1.f)
    ).unit();

    auto& drone = create(meshes.npc_drone_1);

    // @temporary
    drone.position = tVec3f(
      Tachyon_GetRandom(-200000.f, 200000.f),
      Tachyon_GetRandom(-100000.f, 400000.f),
      Tachyon_GetRandom(-200000.f, 200000.f)
    );

    drone.scale = 1000.f;
    drone.rotation = DirectionToQuaternion(direction.invert());

    commit(drone);

    // @todo make sure npc drone lights get restored when exiting the game editor
    point_lights.push_back({
      .position = drone.position,
      .radius = 2000.f,
      .color = tVec3f(1.f, 0.2f, 1.f),
      .power = 1.f
    });

    state.vehicles.push_back({
      .object = drone,
      .position = drone.position,
      .direction = direction,
      .speed = 5000.f,
      .light_index = uint32(point_lights.size() - 1)
    });
  }
}

void Vehicles::UpdateVehicles(Tachyon* tachyon, State& state, const float dt) {
  auto& point_lights = tachyon->point_lights;

  for (auto& vehicle : state.vehicles) {
    auto& object = *get_original_object(vehicle.object);
    auto& light = point_lights[vehicle.light_index];

    // @temporary
    {
      float angle = state.current_game_time * 0.05f + (float)vehicle.object.object_id;

      if (vehicle.object.object_id % 2 == 0) {
        angle *= -1.f;
      }

      object.position.x = 120000.f * cosf(angle);
      object.position.z = 120000.f * sinf(angle);

      if (vehicle.object.object_id % 2 == 0) {
        object.rotation = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), -angle);
      } else {
        object.rotation = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), -angle + t_PI);
      }
    }

    light.position =
      object.position +
      object.rotation.getUpDirection() * 500.f;

    commit(object);
  }
}