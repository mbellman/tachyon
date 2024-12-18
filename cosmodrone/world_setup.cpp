#include <string>
#include <vector>

#include "cosmodrone/mesh_library.h"
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

static void LoadWorldData(Tachyon* tachyon, State& state) {
  auto start_time = Tachyon_GetMicroseconds();
  auto data = Tachyon_GetFileContents("./cosmodrone/data/world.txt");
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

static void InitializeLevel(Tachyon* tachyon, State& state) {
  auto& camera = tachyon->scene.camera;
  auto& meshes = state.meshes;

  // Earth + Moon
  create(meshes.planet);
  create(meshes.planet);

  // Earth atmosphere
  create(meshes.earth_atmosphere);

  // Background space elevator
  create(meshes.space_elevator);

  // Local elevator car
  create(meshes.elevator_car_1);
  create(meshes.elevator_car_1_frame);

  // HUD
  {
    for (uint8 i = 0; i < 16; i++) {
      create(meshes.hud_flight_arrow);
    }

    create(meshes.hud_wedge);
    create(meshes.hud_wedge);
  }

  // @todo define as a default
  state.target_ship_rotation = (
    Quaternion(1.f, 0, 0, 0) *
    Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), t_HALF_PI) *
    Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), -t_HALF_PI) *
    Quaternion::fromAxisAngle(tVec3f(0, 0, 1.f), -t_HALF_PI)
  );
  // @todo define as a default
  state.ship_position = tVec3f(9000.f, 116000.f, -26000.f);

  // Set the initial camera behind the player drone
  state.target_camera_rotation = camera.rotation = state.target_ship_rotation.opposite();

  // @todo improve ship part handling
  {
    auto& hull = create(meshes.hull);
    auto& streams = create(meshes.streams);
    auto& thrusters = create(meshes.thrusters);
    auto& trim = create(meshes.trim);

    hull.scale = 600.f;
    hull.material = tVec4f(0.1f, 0, 0.2f, 0.5f);

    streams.scale = 600.f;
    streams.material = tVec4f(0.5f, 0.5f, 0.5f, 0.2f);

    thrusters.scale = 600.f;
    thrusters.color = tVec3f(0.1f);
    thrusters.material = tVec4f(0.8f, 0, 0, 0.2f);

    trim.scale = 600.f;
    trim.material = tVec4f(0.2f, 1.f, 0, 0);

    // @todo define as a default
    hull.rotation = streams.rotation = thrusters.rotation = trim.rotation = state.target_ship_rotation;

    commit(hull);
    commit(streams);
    commit(thrusters);
    commit(trim);
  }

  LoadWorldData(tachyon, state);
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

static inline void StoreInitialMeshObjects(Tachyon* tachyon, uint16 mesh_index) {
  auto& group = objects(mesh_index);

  group.initial_objects.clear();

  for (auto& object : group) {
    group.initial_objects.push_back(object);
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

void WorldSetup::InitializeGameWorld(Tachyon* tachyon, State& state) {
  InitializeLevel(tachyon, state);

  // @todo dev mode only
  CreateDebugMeshes(tachyon, state);
  CreateEditorGuidelines(tachyon, state);

  RebuildGeneratedObjects(tachyon, state);
  StoreInitialObjects(tachyon, state);
}

void WorldSetup::StoreInitialObjects(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  StoreInitialMeshObjects(tachyon, meshes.elevator_torus_1);
  StoreInitialMeshObjects(tachyon, meshes.elevator_torus_1_frame);

  StoreInitialMeshObjects(tachyon, meshes.station_torus_1);

  StoreInitialMeshObjects(tachyon, meshes.station_torus_2_body);
  StoreInitialMeshObjects(tachyon, meshes.station_torus_2_supports);
  StoreInitialMeshObjects(tachyon, meshes.station_torus_2_frame);

  StoreInitialMeshObjects(tachyon, meshes.station_torus_3_body);
  StoreInitialMeshObjects(tachyon, meshes.station_torus_3_frame);
  StoreInitialMeshObjects(tachyon, meshes.station_torus_3_lights);

  StoreInitialMeshObjects(tachyon, meshes.habitation_4_body);
  StoreInitialMeshObjects(tachyon, meshes.habitation_4_core);
  StoreInitialMeshObjects(tachyon, meshes.habitation_4_frame);
  StoreInitialMeshObjects(tachyon, meshes.habitation_4_panels);
  StoreInitialMeshObjects(tachyon, meshes.habitation_4_lights);
}

void WorldSetup::RebuildGeneratedObjects(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  // Disable placeholder meshes
  for (auto& asset : MeshLibrary::GetPlaceableMeshAssets()) {
    if (asset.placeholder) {
      objects(asset.mesh_index).disabled = true;
    }
  }

  for (auto& asset : MeshLibrary::GetGeneratedMeshAssets()) {
    remove_all(asset.mesh_index);

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

  RebuildLightSources(tachyon, state);
}