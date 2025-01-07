#include <string>
#include <vector>

#include "cosmodrone/mesh_library.h"
#include "cosmodrone/object_behavior.h"
#include "cosmodrone/procedural_generation.h"
#include "cosmodrone/vehicles.h"
#include "cosmodrone/world_setup.h"

using namespace Cosmodrone;

static std::vector<std::string> SplitString(const std::string& str, const std::string& delimiter) {
  std::vector<std::string> values;
  uint32 offset = 0;
  uint32 found = 0;

  // Add each delimited string segment to the list
  while ((found = str.find(delimiter, offset)) != std::string::npos) {
    values.push_back(str.substr(offset, found - offset));

    offset = found + delimiter.size();
  }

  // Include the remaining string segment after the final delimiter
  values.push_back(str.substr(offset, str.size() - offset));

  return values;
}

static const MeshAsset* GetMeshAssetByName(const std::string& mesh_name) {
  auto& assets = MeshLibrary::GetPlaceableMeshAssets();

  for (auto& asset : assets) {
    if (asset.mesh_name == mesh_name) {
      return &asset;
    }
  }

  return nullptr;
}

static void LoadWorldData(Tachyon* tachyon, State& state, const std::string& file) {
  auto start_time = Tachyon_GetMicroseconds();
  auto data = Tachyon_GetFileContents(file.c_str());
  auto lines = SplitString(data, "\n");

  const MeshAsset* mesh_asset;

  for (uint32 i = 0; i < lines.size(); i++) {
    auto& line = lines[i];

    if (line.size() == 0) {
      continue;
    }

    if (line[0] == '@') {
      auto mesh_name = line.substr(1);

      mesh_asset = GetMeshAssetByName(mesh_name);

      // @temporary
      printf("Loading objects: %s\n", mesh_name.c_str());
    } else {
      auto parts = SplitString(line, ",");
      auto& object = create(mesh_asset->mesh_index);

      #define df(n) stof(parts[n])
      #define di(n) stoi(parts[n])

      object.position = tVec3f(df(0), df(1), df(2));
      object.scale = mesh_asset->defaults.scale;
      object.rotation = Quaternion(df(3), df(4), df(5), df(6));
      object.color.rgba = di(7);
      object.material = mesh_asset->defaults.material;

      commit(object);
    }
  }

  auto load_time = Tachyon_GetMicroseconds() - start_time;

  add_console_message("Loaded world data in " + std::to_string(load_time) + "us", tVec3f(1.f));
}

static void InitLevel(Tachyon* tachyon, State& state) {
  auto& camera = tachyon->scene.camera;
  auto& meshes = state.meshes;

  // Earth + Moon
  create(meshes.planet);
  create(meshes.planet);

  // Earth atmosphere
  create(meshes.earth_atmosphere);

  // Background space elevator
  // create(meshes.space_elevator);

  // Local elevator car
  // create(meshes.elevator_car_1);
  // create(meshes.elevator_car_1_frame);

  // HUD
  {
    for (uint8 i = 0; i < 16; i++) {
      create(meshes.hud_flight_arrow);
    }
  }

  // @todo define as a default
  state.target_ship_rotation = (
    Quaternion(1.f, 0, 0, 0) *
    Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), t_HALF_PI) *
    Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), -t_HALF_PI) *
    Quaternion::fromAxisAngle(tVec3f(0, 0, 1.f), -t_HALF_PI)
  );

  // @todo define as a default
  state.ship_position = tVec3f(9000.f, 116000.f, -24000.f);

  // Set the initial camera behind the player drone
  state.target_camera_rotation = camera.rotation = state.target_ship_rotation.opposite();

  // @todo improve ship part handling
  {
    auto& hull = create(meshes.hull);
    auto& streams = create(meshes.streams);
    auto& thrusters = create(meshes.thrusters);
    auto& trim = create(meshes.trim);
    auto& jets = create(meshes.jets);

    hull.scale = 600.f;
    hull.material = tVec4f(0.1f, 0, 0.2f, 0.5f);

    streams.scale = 600.f;
    streams.material = tVec4f(0.4f, 0.5f, 0, 0.2f);

    thrusters.scale = 600.f;
    thrusters.color = tVec3f(0.1f);
    thrusters.material = tVec4f(0.4f, 0, 0, 0.2f);

    trim.scale = 600.f;
    trim.material = tVec4f(0.2f, 1.f, 0, 0);

    jets.scale = 600.f;
    jets.color = tVec4f(0.1f, 0.2f, 1.f, 1.f);

    hull.rotation =
    streams.rotation =
    thrusters.rotation =
    trim.rotation =
    jets.rotation = state.target_ship_rotation;

    commit(hull);
    commit(streams);
    commit(thrusters);
    commit(trim);
    commit(jets);
  }

  #if USE_PROCEDURAL_GENERATION == 1
    LoadWorldData(tachyon, state, "./cosmodrone/data/world_2.txt");
  #else
    LoadWorldData(tachyon, state, "./cosmodrone/data/world.txt");
  #endif
}

static void CreateDebugMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  create(meshes.cube);
  create(meshes.cube);
  create(meshes.cube);

  create(meshes.cube);
  create(meshes.cube);
  create(meshes.cube);
}

// @todo move to editor
static void CreateEditorGuidelines(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  const float width = 500000.f;
  const float height = 1000000.f;
  const float thickness = 20.f;

  // Vertical lines
  for (uint32 i = 0; i <= 10; i++) {
    for (uint32 j = 0; j <= 10; j++) {
      auto& line = create(meshes.editor_guideline);

      auto x = -width + (width / 5.f) * float(i);
      auto z = -width + (width / 5.f) * float(j);

      line.scale = tVec3f(thickness, height, thickness);
      line.color = tVec4f(1.f, 0, 0, 0.2f);
      line.position = tVec3f(x, 0, z);

      commit(line);
    }
  }

  // Planes
  const float y_increment = height / 25.f;
  const float w_increment = width / 5.f;

  for (uint32 i = 0; i < 50; i++) {
    auto y = -height + y_increment * float(i);

    for (uint32 j = 0; j <= 10; j++) {
      auto& line = create(meshes.editor_guideline);

      line.scale = tVec3f(width, thickness, thickness);
      line.position = tVec3f(0, y, -width + w_increment * float(j));
      line.color = tVec4f(1.f, 0, 0, 0.2f);

      commit(line);
    }

    for (uint32 j = 0; j <= 10; j++) {
      auto& line = create(meshes.editor_guideline);

      line.scale = tVec3f(thickness, thickness, width);
      line.position = tVec3f(-width + w_increment * float(j), y, 0);
      line.color = tVec4f(1.f, 0, 0, 0.2f);

      commit(line);
    }
  }
}

static void RebuildLightSources(Tachyon* tachyon, State& state) {
  auto& point_lights = tachyon->point_lights;

  state.gas_flare_light_indexes.clear();
  // @todo only clear generated lights
  tachyon->point_lights.clear();

  for (auto& bulb : objects(state.meshes.light_1_bulb)) {
    tVec3f offset = bulb.rotation.toMatrix4f() * tVec3f(0.f, 0.1f, 0);
    offset *= bulb.scale;

    point_lights.push_back({
      .position = bulb.position + offset,
      .radius = 2000.f,
      .color = tVec3f(1.f, 0.3f, 0.1f),
      .power = 1.f
    });
  }

  for (auto& bulb : objects(state.meshes.light_2_bulb)) {
    tVec3f offset = bulb.rotation.toMatrix4f() * tVec3f(0.f, 0.1f, 0);
    offset *= bulb.scale;

    point_lights.push_back({
      .position = bulb.position + offset,
      .radius = 2000.f,
      .color = tVec3f(0.1f, 0.4f, 1.f),
      .power = 1.f
    });
  }

  for (auto& bulb : objects(state.meshes.light_3_bulb)) {
    tVec3f offset = bulb.rotation.toMatrix4f() * tVec3f(0.f, 0.1f, 0);
    offset *= bulb.scale;

    point_lights.push_back({
      .position = bulb.position + offset,
      .radius = 7000.f,
      .color = tVec3f(0.5, 1.f, 0.8f),
      .power = 1.f
    });
  }

  for (auto& flare : objects(state.meshes.gas_flare_1_spawn)) {
    point_lights.push_back({
      .position = flare.position,
      .radius = 50000.f,
      .color = tVec3f(1.f, 0.5f, 0.1f),
      .power = 5.f
    });

    state.gas_flare_light_indexes.push_back(point_lights.size() - 1);
  }
}

static void RebuildBeacons(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  state.beacons.clear();

  remove_all(meshes.beacon);

  // @todo factor
  for (auto& object : objects(meshes.antenna_3)) {
    Beacon beacon;
    beacon.beacon_1 = create(meshes.beacon);
    beacon.beacon_2 = create(meshes.beacon);
    beacon.source_object = object;

    beacon.beacon_1.rotation = object.rotation;
    beacon.beacon_2.rotation = object.rotation;
    beacon.beacon_1.scale = 1500.f;
    beacon.beacon_2.scale = 1500.f;
    beacon.beacon_1.color = tVec4f(1.f, 0.5f, 0.2f, 1.f);
    beacon.beacon_2.color = tVec4f(1.f, 0.5f, 0.2f, 1.f);

    state.beacons.push_back(beacon);
  }

  // @todo factor
  for (auto& object : objects(meshes.charge_pad)) {
    Beacon beacon;
    beacon.beacon_1 = create(meshes.beacon);
    beacon.beacon_2 = create(meshes.beacon);
    beacon.source_object = object;

    beacon.beacon_1.rotation = object.rotation;
    beacon.beacon_2.rotation = object.rotation;
    beacon.beacon_1.scale = 1500.f;
    beacon.beacon_2.scale = 1500.f;
    beacon.beacon_1.color = tVec4f(1.f, 0.5f, 0.2f, 1.f);
    beacon.beacon_2.color = tVec4f(1.f, 0.5f, 0.2f, 1.f);

    state.beacons.push_back(beacon);
  }
}

static void DisablePlaceholderMeshes(Tachyon* tachyon) {
  for (auto& asset : MeshLibrary::GetPlaceableMeshAssets()) {
    if (asset.placeholder) {
      objects(asset.mesh_index).disabled = true;
    }
  }
}

static void RebuildGeneratedObjects(Tachyon* tachyon) {
  for (auto& asset : MeshLibrary::GetGeneratedMeshAssets()) {
    for (auto& base : objects(asset.generated_from)) {
      auto& piece = create(asset.mesh_index);

      piece.position = base.position;

      if (asset.defaults.scale.x == 1000.f) {
        piece.scale = base.scale;
      } else {
        piece.scale = asset.defaults.scale;
      }

      piece.rotation = base.rotation;

      piece.color = asset.defaults.color;
      piece.material = asset.defaults.material;

      commit(piece);
    }

    objects(asset.mesh_index).disabled = false;
  }
}

void WorldSetup::InitWorld(Tachyon* tachyon, State& state) {
  InitLevel(tachyon, state);
  RebuildWorld(tachyon, state);

  // @todo dev mode only
  {
    CreateDebugMeshes(tachyon, state);
    CreateEditorGuidelines(tachyon, state);
  }
}

void WorldSetup::RebuildWorld(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  for (auto& asset : MeshLibrary::GetGeneratedMeshAssets()) {
    remove_all(asset.mesh_index);
  }

  #if USE_PROCEDURAL_GENERATION == 1
    ProceduralGeneration::GenerateWorld(tachyon, state);
  #endif

  DisablePlaceholderMeshes(tachyon);
  RebuildGeneratedObjects(tachyon);

  RebuildLightSources(tachyon, state);
  RebuildBeacons(tachyon, state);

  ObjectBehavior::InitObjects(tachyon, state);
  Vehicles::InitVehicles(tachyon, state);
}