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

  create(meshes.planet);
  create(meshes.planet);
  create(meshes.space_elevator);

  // @todo improve ship part handling
  {
    auto& hull = create(meshes.hull);
    auto& streams = create(meshes.streams);
    auto& thrusters = create(meshes.thrusters);
    auto& trim = create(meshes.trim);

    hull.scale = 200.f;
    hull.material = tVec4f(0.8f, 1.f, 0.2f, 0);

    streams.scale = 200.f;
    streams.material = tVec4f(0.6f, 0, 0, 1.f);

    thrusters.scale = 200.f;
    thrusters.color = tVec3f(0.2f);
    thrusters.material = tVec4f(0.8f, 0, 0, 0.4f);

    trim.scale = 200.f;
    trim.material = tVec4f(0.2f, 1.f, 0, 0);

    hull.rotation = streams.rotation = thrusters.rotation = trim.rotation = (
      Quaternion(1.f, 0, 0, 0) *
      Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), -t_HALF_PI)
    );

    commit(hull);
    commit(streams);
    commit(thrusters);
    commit(trim);
  }

  camera.rotation = Quaternion(0.707f, 0.707f, 0, 0);
  state.target_camera_rotation = camera.rotation;

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

void WorldSetup::InitializeGameWorld(Tachyon* tachyon, State& state) {
  InitializeLevel(tachyon, state);

  // @todo dev mode only
  CreateDebugMeshes(tachyon, state);
  CreateEditorGuidelines(tachyon, state);

  StoreInitialObjects(tachyon, state);
  RebuildGeneratedObjects(tachyon, state);
}

void WorldSetup::StoreInitialObjects(Tachyon* tachyon, State& state) {
  #define store(__mesh_index)\
    auto& group = objects(__mesh_index);\
    group.initial_objects.clear();\
    for (auto& object : objects(__mesh_index)) {\
      group.initial_objects.push_back(object);\
    };\

  store(state.meshes.station_torus_1);
}

// @todo refactor
void WorldSetup::RebuildGeneratedObjects(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  // antenna_2
  {
    remove_all(meshes.antenna_2_frame);
    remove_all(meshes.antenna_2_receivers);

    for (auto& antenna : objects(meshes.antenna_2)) {
      auto& frame = create(meshes.antenna_2_frame);
      auto& receivers = create(meshes.antenna_2_receivers);

      frame.position = receivers.position = antenna.position;
      frame.scale = receivers.scale = antenna.scale;
      frame.rotation = receivers.rotation = antenna.rotation;

      frame.color = antenna.color;
      frame.material = antenna.material;

      receivers.color = tVec3f(1.f);
      receivers.material = tVec4f(0.9f, 0, 0, 0.2f);

      commit(frame);
      commit(receivers);
    }

    objects(meshes.antenna_2).disabled = true;

    objects(meshes.antenna_2_frame).disabled = false;
    objects(meshes.antenna_2_receivers).disabled = false;
  }

  // girder_6
  {
    remove_all(meshes.girder_6_core);
    remove_all(meshes.girder_6_frame);

    for (auto& girder : objects(meshes.girder_6)) {
      auto& core = create(meshes.girder_6_core);
      auto& frame = create(meshes.girder_6_frame);

      core.position = frame.position = girder.position;
      core.scale = frame.scale = girder.scale;
      core.rotation = frame.rotation = girder.rotation;

      core.color = girder.color;
      core.material = girder.material;

      frame.color = tVec3f(1.f);
      frame.material = tVec4f(0.4f, 1.f, 0, 0);

      commit(core);
      commit(frame);
    }

    objects(meshes.girder_6).disabled = true;

    objects(meshes.girder_6_core).disabled = false;
    objects(meshes.girder_6_frame).disabled = false;
  }

  // module_2
  {
    remove_all(meshes.module_2_core);
    remove_all(meshes.module_2_frame);

    for (auto& module : objects(meshes.module_2)) {
      auto& core = create(meshes.module_2_core);
      auto& frame = create(meshes.module_2_frame);

      core.position = frame.position = module.position;
      core.scale = frame.scale = module.scale;
      core.rotation = frame.rotation = module.rotation;

      core.color = module.color;
      core.material = module.material;

      frame.color = tVec3f(1.f);
      frame.material = tVec4f(0.4f, 1.f, 0, 0);

      commit(core);
      commit(frame);
    }

    objects(meshes.module_2).disabled = true;

    objects(meshes.module_2_core).disabled = false;
    objects(meshes.module_2_frame).disabled = false;
  }
}