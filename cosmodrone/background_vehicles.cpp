#include "cosmodrone/background_vehicles.h"
#include "cosmodrone/mesh_library.h"

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

static void SpawnFlyingShips(Tachyon* tachyon, State& state) {
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
        .parts = { ship },
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

// @todo accept position/rotation
static void SpawnMovingCargoFerry(Tachyon* tachyon, const State& state, BackgroundVehicle& vehicle) {
  auto& meshes = state.meshes;

  #define set_attributes(object, asset)\
    {\
      auto& defaults = MeshLibrary::FindMeshAsset(asset).defaults;\
      object.color = defaults.color;\
      object.material = defaults.material;\
    }\

  auto& core = create(meshes.freight_core);
  auto& frame = create(meshes.freight_frame);
  auto& thrusters = create(meshes.freight_thrusters);
  auto& dock = create(meshes.freight_dock);
  auto& jets = create(meshes.freight_jets);

  set_attributes(core, meshes.freight_core);
  set_attributes(frame, meshes.freight_frame);
  set_attributes(thrusters, meshes.freight_thrusters);
  set_attributes(dock, meshes.freight_dock);
  set_attributes(jets, meshes.freight_jets);

  core.scale =
  frame.scale =
  thrusters.scale =
  dock.scale =
  jets.scale = 8000.f;

  jets.color.rgba |= 0x000F;

  vehicle.parts = { core, frame, thrusters, dock, jets };
}

static void SpawnMovingCargoFerries(Tachyon* tachyon, State& state) {
  for (auto& node : objects(state.meshes.freight_vehicle_target)) {
    for (auto& node2 : objects(state.meshes.freight_vehicle_target)) {
      if (node == node2) {
        continue;
      }

      BackgroundVehicle vehicle;

      SpawnMovingCargoFerry(tachyon, state, vehicle);

      vehicle.spawn_position = node.position;
      vehicle.target_position = node2.position;

      // @temporary
      for (auto& part : vehicle.parts) {
        auto& live_part = *get_live_object(part);

        live_part.position = vehicle.spawn_position;

        commit(live_part);
      }

      state.vehicles.push_back(vehicle);
    }
  }
}

static void UpdateFlyingShip(Tachyon* tachyon, BackgroundVehicle& vehicle, const float dt) {
  auto& point_lights = tachyon->point_lights;
  auto& ship = *get_live_object(vehicle.parts[0]);
  auto object_to_target = vehicle.target_position - ship.position;
  auto distance = object_to_target.magnitude();
  auto distance_to_camera = (ship.position - tachyon->scene.camera.position).magnitude();
  auto direction = object_to_target / distance;

  if (distance < 10000.f) {
    ship.position = vehicle.spawn_position;
  }

  ship.position += direction * 25000.f * dt;
  ship.rotation = DirectionToQuaternion(direction);

  auto& light1 = point_lights[vehicle.light_indexes_offset];
  auto& light2 = point_lights[vehicle.light_indexes_offset + 1];
  auto matrix = ship.rotation.toMatrix4f();

  float light_ratio = distance_to_camera / 2000000.f;
  if (light_ratio > 1.f) light_ratio = 1.f;
  light_ratio *= light_ratio;

  light1.position =
    ship.position +
    matrix.transformVec3f(tVec3f(-0.2f, 0, -0.95f) * ship.scale);

  light2.position =
    ship.position +
    matrix.transformVec3f(tVec3f(0.2f, 0, -0.95f) * ship.scale);

  light1.power =
  light2.power =
  1.f + 50.f * light_ratio;

  light1.radius =
  light2.radius =
  2000.f + 20000.f * light_ratio;

  commit(ship);
}

static void UpdateCargoFerry(Tachyon* tachyon, BackgroundVehicle& vehicle, const float dt) {
  auto& ship = *get_live_object(vehicle.parts[0]);
  auto object_to_target = vehicle.target_position - ship.position;
  auto distance = object_to_target.magnitude();
  auto direction = object_to_target / distance;

  if (distance < 10000.f) {
    ship.position = vehicle.spawn_position;
  }

  ship.position += direction * 50000.f * dt;
  ship.rotation = DirectionToQuaternion(direction.invert());

  // @todo only apply position/rotation to additional parts
  for (auto& part : vehicle.parts) {
    auto& live_part = *get_live_object(part);

    live_part.position = ship.position;
    live_part.rotation = ship.rotation;

    commit(live_part);
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
  SpawnFlyingShips(tachyon, state);
  SpawnMovingCargoFerries(tachyon, state);

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
    auto& main_part = vehicle.parts[0];

    if (main_part.mesh_index == state.meshes.flying_ship_1) {
      UpdateFlyingShip(tachyon, vehicle, dt);
    }

    if (main_part.mesh_index == state.meshes.freight_core) {
      UpdateCargoFerry(tachyon, vehicle, dt);
    }
  }

  auto t = Tachyon_GetMicroseconds() - start;

  // @todo dev mode only
  add_dev_label("UpdateVehicles", std::to_string(t) + "us");
}