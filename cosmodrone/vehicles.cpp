#include "cosmodrone/vehicles.h"

using namespace Cosmodrone;

// @todo move to constants
const static auto FORWARD_VECTOR = tVec3f(0, 0, -1.f);
const static auto UP_VECTOR = tVec3f(0, 1.f, 0);
const static auto RIGHT_VECTOR = tVec3f(1.f, 0, 0);

// @todo move to engine
static inline float Lerpf(float a, float b, float alpha) {
  return a + (b - a) * alpha;
}

// @todo move to engine
static inline tVec3f Lerpf(const tVec3f& a, const tVec3f& b, const float alpha) {
  return tVec3f(
    Lerpf(a.x, b.x, alpha),
    Lerpf(a.y, b.y, alpha),
    Lerpf(a.z, b.z, alpha)
  );
}

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

  meshes.flying_car_1 = Tachyon_AddMesh(
    tachyon,
    Tachyon_LoadMesh("./cosmodrone/assets/npc-ships/flying_car_1.obj"),
    500
  );
}

void Vehicles::InitVehicles(Tachyon* tachyon, State& state) {
  auto& point_lights = tachyon->point_lights;
  auto& vehicles = state.vehicles;
  auto& network = state.vehicle_network;
  auto& meshes = state.meshes;

  // Reset vehicle instances/objects
  {
    vehicles.clear();
    network.clear();

    remove_all(meshes.npc_drone_1);
    remove_all(meshes.flying_car_1);
  }

  // Build vehicle network
  {
    for (auto& target : objects(meshes.vehicle_target)) {
      VehicleNode node;

      node.position = target.position;

      network.push_back(node);
    }

    for (auto& node : network) {
      for (auto& node2 : network) {
        float distance = (node.position - node2.position).magnitude();

        if (distance > 10000.f && distance < 1700000.f) {
          node.connected_nodes.push_back(node2);
        }
      }
    }
  }

  // npc_drone_1
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

    point_lights.push_back({
      .position = drone.position,
      .radius = 2000.f,
      .color = tVec3f(1.f, 0.2f, 1.f),
      .power = 1.f
    });

    vehicles.push_back({
      .object = drone,
      // @temporary
      .spawn_position = tVec3f(0.f),
      // @temporary
      .target_position = tVec3f(0.f),
      .speed = 5000.f,
      .light_index = uint32(point_lights.size() - 1)
    });
  }

  // flying_car_1
  for (auto& node : state.vehicle_network) {
    uint32 spawn_total = node.connected_nodes.size() * 5;

    for (uint32 i = 0; i < spawn_total; i++) {
      auto& ship = create(meshes.flying_car_1);
      auto target_node_index = (int)(Tachyon_GetRandom() * node.connected_nodes.size());
      auto& target_node = node.connected_nodes[target_node_index];
      float progress = Tachyon_GetRandom();

      ship.position = Lerpf(node.position, target_node.position, progress);
      ship.scale = 8000.f;

      commit(ship);

      Vehicle vehicle;
      vehicle.object = ship;
      vehicle.spawn_position = node.position;
      vehicle.target_position = target_node.position;

      vehicles.push_back(vehicle);
    }
  }

  // @todo dev mode only
  console_log(
    "Spawned " +
    std::to_string(objects(meshes.flying_car_1).total_active) +
    " flying cars"
  );
}

void Vehicles::UpdateVehicles(Tachyon* tachyon, State& state, const float dt) {
  auto& point_lights = tachyon->point_lights;

  for (auto& vehicle : state.vehicles) {
    auto& object = *get_original_object(vehicle.object);

    // @temporary
    if (object.mesh_index == state.meshes.npc_drone_1) {
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

    if (object.mesh_index == state.meshes.flying_car_1) {
      auto object_to_target = vehicle.target_position - object.position;
      auto distance = object_to_target.magnitude();
      auto direction = object_to_target / distance;

      if (distance < 10000.f) {
        object.position = vehicle.spawn_position;
      }

      object.position += direction * 10000.f * dt;
      object.rotation = DirectionToQuaternion(direction);
    }

    if (vehicle.light_index > 0) {
      auto& light = point_lights[vehicle.light_index];

      light.position =
        object.position +
        object.rotation.getUpDirection() * 500.f;
    }

    commit(object);
  }
}