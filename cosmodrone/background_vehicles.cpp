#include "cosmodrone/background_vehicles.h"

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

static void RebuildVehicleNetwork(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;
  auto& network = state.vehicle_network;

  // Create a network node for each vehicle target object
  for (auto& target : objects(meshes.vehicle_target)) {
    VehicleNetworkNode node;

    node.position = target.position;

    network.push_back(node);
  }

  // Connect nodes within range
  for (auto& node : network) {
    for (auto& node2 : network) {
      float distance = (node.position - node2.position).magnitude();

      if (distance > 10000.f && distance < 1700000.f) {
        node.connected_nodes.push_back(node2);
      }
    }
  }
}

static void RecreateFlyingShips(Tachyon* tachyon, State& state) {
  auto& point_lights = tachyon->point_lights;
  auto& meshes = state.meshes;
  auto& vehicles = state.vehicles;

  for (auto& node : state.vehicle_network) {
    uint32 spawn_total = node.connected_nodes.size() * 3;

    for (uint32 i = 0; i < spawn_total; i++) {
      auto& ship = create(meshes.flying_ship_1);
      auto target_node_index = (int)(Tachyon_GetRandom() * node.connected_nodes.size());
      auto& target_node = node.connected_nodes[target_node_index];
      float progress = Tachyon_GetRandom();

      tVec3f offset = tVec3f(
        Tachyon_GetRandom(0.f, 40000.f),
        Tachyon_GetRandom(0.f, 40000.f),
        Tachyon_GetRandom(0.f, 40000.f)
      );

      ship.position = tVec3f::lerp(node.position, target_node.position, progress) + offset;
      ship.scale = 8000.f;

      commit(ship);

      vehicles.push_back({
        .object = ship,
        .spawn_position = node.position + offset,
        .target_position = target_node.position + offset,
        .light_indexes_offset = uint32(point_lights.size())
      });

      point_lights.push_back({
        .position = ship.position,
        .radius = 2000.f,
        .color = tVec3f(0.2f, 0.6f, 1.f),
        .power = 2.f
      });

      point_lights.push_back({
        .position = ship.position,
        .radius = 2000.f,
        .color = tVec3f(0.2f, 0.6f, 1.f),
        .power = 2.f
      });
    }
  }
}

void BackgroundVehicles::LoadVehicleMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  meshes.flying_ship_1 = Tachyon_AddMesh(
    tachyon,
    Tachyon_LoadMesh("./cosmodrone/assets/npc-ships/flying_ship_1.obj"),
    1000
  );
}

void BackgroundVehicles::InitVehicles(Tachyon* tachyon, State& state) {
  auto& point_lights = tachyon->point_lights;
  auto& vehicles = state.vehicles;
  auto& network = state.vehicle_network;
  auto& meshes = state.meshes;

  // Reset vehicle instances/objects
  {
    vehicles.clear();
    network.clear();

    remove_all(meshes.flying_ship_1);
  }

  RebuildVehicleNetwork(tachyon, state);
  RecreateFlyingShips(tachyon, state);

  // @todo dev mode only
  {
    auto total_flying_ships = objects(meshes.flying_ship_1).total_active;

    console_log("Spawned " + std::to_string(total_flying_ships) + " flying cars");
  }
}

void BackgroundVehicles::UpdateVehicles(Tachyon* tachyon, State& state, const float dt) {
  auto start = Tachyon_GetMicroseconds();
  auto& point_lights = tachyon->point_lights;

  for (auto& vehicle : state.vehicles) {
    auto& object = *get_original_object(vehicle.object);

    // Flying ships
    if (object.mesh_index == state.meshes.flying_ship_1) {
      auto object_to_target = vehicle.target_position - object.position;
      auto distance = object_to_target.magnitude();
      auto direction = object_to_target / distance;

      if (distance < 10000.f) {
        object.position = vehicle.spawn_position;
      }

      object.position += direction * 25000.f * dt;
      object.rotation = DirectionToQuaternion(direction);

      auto& light1 = point_lights[vehicle.light_indexes_offset];
      auto& light2 = point_lights[vehicle.light_indexes_offset + 1];
      auto matrix = object.rotation.toMatrix4f();

      float light_ratio = distance / 2000000.f;
      if (light_ratio > 1.f) light_ratio = 1.f;
      light_ratio *= light_ratio;

      light1.position =
        object.position +
        matrix.transformVec3f(tVec3f(-0.2f, 0, -0.95f) * object.scale);

      light2.position =
        object.position +
        matrix.transformVec3f(tVec3f(0.2f, 0, -0.95f) * object.scale);

      light1.power =
      light2.power =
      1.f + 50.f * light_ratio;

      light1.radius =
      light2.radius =
      1000.f + 20000.f * light_ratio;
    }

    commit(object);
  }

  auto t = Tachyon_GetMicroseconds() - start;

  // @todo dev mode only
  add_dev_label("UpdateVehicles", std::to_string(t) + "us");
}