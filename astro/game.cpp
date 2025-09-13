#include "astro/game.h"
#include "astro/entity_manager.h"
#include "astro/mesh_library.h"
#include "astro/object_manager.h"
#include "astro/time_evolution.h"

using namespace astro;

static void UpdatePlayer(Tachyon* tachyon, State& state) {
  auto& player = objects(state.meshes.player)[0];
  
  player.position = state.player_position;
  // @temporary
  player.scale = tVec3f(600.f);
  player.scale.y = 1200.f;
  player.color = tVec3f(0, 0.2f, 1.f);

  commit(player);
}

static void UpdateWaterPlane(Tachyon* tachyon, State& state) {
  auto& water_plane = objects(state.meshes.water_plane)[0];

  // @temporary
  water_plane.position = tVec3f(0, -2500.f, 0);
  water_plane.scale = tVec3f(40000.f, 1.f, 40000.f);
  water_plane.color = tVec3f(0, 0.1f, 0.3f);
  water_plane.material = tVec4f(0.1f, 1.f, 0, 0.5f);

  commit(water_plane);
}

static void UpdateGroundPlane(Tachyon* tachyon, State& state) {
  auto& ground_plane = objects(state.meshes.ground_plane)[0];

  // @temporary
  ground_plane.position = tVec3f(0, -1500.f, 2500.f);
  ground_plane.scale = tVec3f(20000.f, 1.f, 5000.f);
  ground_plane.color = tVec3f(0.4f, 0.5f, 0.1f);

  commit(ground_plane);
}

static void ShowGameStats(Tachyon* tachyon, State& state) {
  std::string stat_messages[] = {
    "Player " + state.player_position.toString(),
    "Camera " + tachyon->scene.camera.position.toString(),
    "Astro time: " + std::to_string(state.astro_time)
  };

  for (uint8 i = 0; i < std::size(stat_messages); i++) {
    Tachyon_DrawUIText(tachyon, state.debug_text, {
      .screen_x = tachyon->window_width - 570,
      .screen_y = 20 + (i * 25),
      .centered = false,
      .string = stat_messages[i]
    });
  }
}

static void HandleControls(Tachyon* tachyon, State& state, const float dt) {
  // Handle movement actions
  // @temporary
  {
    if (is_key_held(tKey::ARROW_UP) || is_key_held(tKey::W)) {
      state.player_position += tVec3f(0, 0, -1.f) * 6000.f * dt;
    }

    if (is_key_held(tKey::ARROW_LEFT) || is_key_held(tKey::A)) {
      state.player_position += tVec3f(-1.f, 0, 0) * 6000.f * dt;
    }

    if (is_key_held(tKey::ARROW_RIGHT) || is_key_held(tKey::D)) {
      state.player_position += tVec3f(1.f, 0, 0) * 6000.f * dt;
    }

    if (is_key_held(tKey::ARROW_DOWN) || is_key_held(tKey::S)) {
      state.player_position += tVec3f(0, 0, 1.f) * 6000.f * dt;
    }

    state.player_position.x += tachyon->left_stick.x * 6000.f * dt;
    state.player_position.z += tachyon->left_stick.y * 6000.f * dt;
  }

  // Handle astro time actions
  {
    state.astro_time -= tachyon->left_trigger * 100.f * dt;

    // @todo allow > 0.f astro time changes after obtaining the appropriate item
    if (state.astro_time < 0.f) {
      state.astro_time += tachyon->right_trigger * 100.f * dt;
    }
  }
}

static void ProvisionAvailableObjectsForEntities(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  // @todo @optimize determine on-screen/in-range entities
  // and use reduced-fidelity object groups, or single objects,
  // for more distant entities

  // @todo refactor
  for (int i = 0; i < state.oak_trees.size(); i++) {
    auto& tree = state.oak_trees[i];
    auto& trunk = objects(meshes.oak_tree_trunk)[i];
    float tree_age = state.astro_time - tree.astro_time_when_born;

    trunk.position = tree.position;
    trunk.rotation = tree.orientation;
    trunk.color = tree.tint;
  }

  // @todo refactor
  for (int i = 0; i < state.willow_trees.size(); i++) {
    auto& tree = state.oak_trees[i];
    auto& trunk = objects(meshes.willow_tree_trunk)[i];

    trunk.position = tree.position;
    trunk.rotation = tree.orientation;
    trunk.color = tree.tint;
  }
}

void astro::InitGame(Tachyon* tachyon, State& state) {
  MeshLibrary::AddMeshes(tachyon, state);

  // @todo move to ui.cpp
  state.debug_text = Tachyon_CreateUIText("./fonts/CascadiaMonoNF.ttf", 20);

  Tachyon_InitializeObjects(tachyon);

  ObjectManager::CreateObjects(tachyon, state);

  // @temporary
  {
    auto record = EntityManager::CreateEntity(state, OAK_TREE);
    auto& oak = *(TreeEntity*)EntityManager::FindEntity(state, record);

    oak.position = tVec3f(-5000.f, 0, 0);
    oak.scale = tVec3f(2000.f);
    oak.tint = tVec3f(1.f, 0.6f, 0.3f);

    ObjectManager::CreateObjectsForEntity(tachyon, state, record.type);
  }

  // @temporary
  {
    auto record = EntityManager::CreateEntity(state, OAK_TREE);
    auto& oak = *(TreeEntity*)EntityManager::FindEntity(state, record);

    oak.position = tVec3f(5500.f, 0, 2500.f);
    oak.scale = tVec3f(2500.f);
    oak.tint = tVec3f(1.f, 0.6f, 0.3f);

    ObjectManager::CreateObjectsForEntity(tachyon, state, record.type);
  }
}

void astro::UpdateGame(Tachyon* tachyon, State& state, const float dt) {
  auto& scene = tachyon->scene;

  HandleControls(tachyon, state, dt);

  UpdatePlayer(tachyon, state);
  UpdateWaterPlane(tachyon, state);
  UpdateGroundPlane(tachyon, state);
  
  ProvisionAvailableObjectsForEntities(tachyon, state);

  TimeEvolution::HandleAstroTime(tachyon, state, dt);

  // @todo move to ui.cpp
  // @todo debug mode only
  ShowGameStats(tachyon, state);

  // @temporary
  // @todo unit() this in the renderer
  scene.primary_light_direction = tVec3f(1.f, -1.f, -0.2f).unit();
  scene.camera.position = tVec3f(0, 10000.f, 10000.f);
  scene.camera.rotation = Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), 0.9f);
}