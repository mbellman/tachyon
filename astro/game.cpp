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
  player.scale = tVec3f(600.f, 1500.f, 600.f);
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

// @todo move to its own file
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

// @todo move to its own file
static void UpdateCamera(Tachyon* tachyon, State& state, const float dt) {
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

  camera.position = tVec3f::lerp(camera.position, new_camera_position, 5.f * dt);

  camera.rotation = Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), 0.9f);
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

// @todo cleanup
// @todo move elsewhere
static void UpdateAstrolabe(Tachyon* tachyon, State& state) {
  auto& camera = tachyon->scene.camera;
  auto& meshes = state.meshes;

  auto& base = objects(meshes.astrolabe_base)[0];
  auto& ring = objects(meshes.astrolabe_ring)[0];
  auto& hand = objects(meshes.astrolabe_hand)[0];

  base.scale =
  ring.scale =
  hand.scale =
  200.f;

  base.color = tVec3f(1.f, 0.6f, 0.2f);
  base.material = tVec4f(0.1f, 1.f, 0, 0.5f);

  ring.color = tVec3f(0.2f, 0.4f, 1.f);
  ring.material = tVec4f(0.2f, 1.f, 0, 0);

  hand.color = tVec3f(1.f, 0.6f, 0.2f);
  hand.material = tVec4f(0.1f, 1.f, 0, 1.f);

  base.rotation =
  ring.rotation =
  (
    Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), -0.9f) *
    Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), -t_HALF_PI * 0.85f)
  );

  hand.rotation =
  (
    base.rotation *
    Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), -state.astro_time * 0.02f)
  );

  ring.rotation =
  (
    base.rotation *
    Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), -state.astro_time * 0.015f)
  );

  base.position =
  ring.position =
  hand.position =
  (
    camera.position +
    camera.rotation.getDirection() * tVec3f(1.f, -1.f, 1.f) * 2000.f +
    camera.rotation.getLeftDirection() * 1200.f +
    camera.rotation.getUpDirection() * tVec3f(1.f, -1.f, 1.f) * 600.f
  );

  commit(base);
  commit(ring);
  commit(hand);
}

// @todo move to its own file
static void HandleControls(Tachyon* tachyon, State& state, const float dt) {
  // Handle movement actions
  // @todo refactor
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
    const float astro_turn_rate = 0.2f;
    const float astro_slowdown_rate = 1.f;

    // Handle reverse/forward turn actions
    state.astro_turn_speed -= tachyon->left_trigger * astro_turn_rate * dt;
    state.astro_turn_speed += tachyon->right_trigger * astro_turn_rate * dt;

    // Disable forward time changes past 0.
    // @todo allow this once the appropriate item is obtained
    if (state.astro_time > 0.f && state.astro_turn_speed > 0.f) {
      state.astro_turn_speed = 0.f;
    }

    state.astro_time += state.astro_turn_speed * 100.f * dt;

    // Reduce turn rate gradually
    state.astro_turn_speed *= 1.f - astro_slowdown_rate * dt;

    // Stop turning at a low enough speed
    if (abs(state.astro_turn_speed) < 0.001f) {
      state.astro_turn_speed = 0.f;
    }
  }
}

void astro::InitGame(Tachyon* tachyon, State& state) {
  MeshLibrary::AddMeshes(tachyon, state);

  // @todo move to ui.cpp
  state.debug_text = Tachyon_CreateUIText("./fonts/CascadiaMonoNF.ttf", 20);

  Tachyon_InitializeObjects(tachyon);

  ObjectManager::CreateObjects(tachyon, state);

  // @todo default this somewhere, or load in from save
  tachyon->scene.camera.position = tVec3f(0.f, 10000.f, 10000.f);

  // @temporary
  {
    auto record = EntityManager::CreateEntity(state, OAK_TREE);
    auto& oak = *(TreeEntity*)EntityManager::FindEntity(state, record);

    oak.position = tVec3f(-5000.f, 0, 0);
    oak.scale = tVec3f(2000.f);
    oak.tint = tVec3f(1.f, 0.6f, 0.3f);
    oak.astro_time_when_born = -250.f;

    ObjectManager::CreateObjectsForEntity(tachyon, state, record.type);
  }

  // @temporary
  {
    auto record = EntityManager::CreateEntity(state, OAK_TREE);
    auto& oak = *(TreeEntity*)EntityManager::FindEntity(state, record);

    oak.position = tVec3f(-6000.f, 0, 6000.f);
    oak.scale = tVec3f(2000.f);
    oak.tint = tVec3f(1.f, 0.6f, 0.3f);
    oak.astro_time_when_born = -300.f;

    ObjectManager::CreateObjectsForEntity(tachyon, state, record.type);
  }

  // @temporary
  {
    auto record = EntityManager::CreateEntity(state, OAK_TREE);
    auto& oak = *(TreeEntity*)EntityManager::FindEntity(state, record);

    oak.position = tVec3f(5500.f, 0, 1000.f);
    oak.scale = tVec3f(2500.f);
    oak.tint = tVec3f(1.f, 0.6f, 0.3f);
    oak.astro_time_when_born = -280.f;

    ObjectManager::CreateObjectsForEntity(tachyon, state, record.type);
  }

  // @temporary
  {
    auto record = EntityManager::CreateEntity(state, SHRUB);
    auto& shrub = *(PlantEntity*)EntityManager::FindEntity(state, record);

    shrub.position = tVec3f(6500.f, -1500.f, 3500.f);
    shrub.scale = tVec3f(600.f);
    shrub.tint = tVec3f(0.2f, 0.8f, 0.5f);
    shrub.astro_time_when_born = -50.f;

    ObjectManager::CreateObjectsForEntity(tachyon, state, record.type);
  }

  // @temporary
  {
    auto record = EntityManager::CreateEntity(state, SHRUB);
    auto& shrub = *(PlantEntity*)EntityManager::FindEntity(state, record);

    shrub.position = tVec3f(7000.f, -1500.f, 5200.f);
    shrub.scale = tVec3f(800.f);
    shrub.tint = tVec3f(0.2f, 0.8f, 0.5f);
    shrub.astro_time_when_born = -40.f;

    ObjectManager::CreateObjectsForEntity(tachyon, state, record.type);
  }
}

void astro::UpdateGame(Tachyon* tachyon, State& state, const float dt) {
  auto& scene = tachyon->scene;

  HandleControls(tachyon, state, dt);

  UpdatePlayer(tachyon, state);
  UpdateWaterPlane(tachyon, state);
  UpdateGroundPlane(tachyon, state);
  UpdateCamera(tachyon, state, dt);
  UpdateAstrolabe(tachyon, state);

  ObjectManager::ProvisionAvailableObjectsForEntities(tachyon, state);
  TimeEvolution::HandleAstroTime(tachyon, state, dt);

  // @todo move to ui.cpp
  // @todo debug mode only
  ShowGameStats(tachyon, state);
}