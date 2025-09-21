#include "astro/game.h"
#include "astro/data_loader.h"
#include "astro/entity_dispatcher.h"
#include "astro/entity_manager.h"
#include "astro/level_editor.h"
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
  water_plane.position = tVec3f(0, -3000.f, 0);
  water_plane.scale = tVec3f(40000.f, 1.f, 40000.f);
  water_plane.color = tVec3f(0, 0.1f, 0.3f);
  water_plane.material = tVec4f(0.1f, 1.f, 0, 0.5f);

  commit(water_plane);
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

  // @temporary
  int32 room_x = (int32)roundf(state.player_position.x / 16000.f);
  int32 room_z = (int32)roundf(state.player_position.z / 16000.f);

  if (room_x != 0 || room_z != 0) {
    new_camera_position = state.player_position;
    new_camera_position.y += 10000.f;
    new_camera_position.z += 7000.f;
  }

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

static void HandleCollisions(Tachyon* tachyon, State& state) {
  auto& player_position = state.player_position;

  for_entities(state.shrubs) {
    auto& shrub = state.shrubs[i];
    auto& position = shrub.position;

    float radius = shrub.visible_scale.x > shrub.visible_scale.z
      ? 2.f * shrub.visible_scale.x
      : 2.f * shrub.visible_scale.z;

    // Skip small or invisible shrubs
    if (radius < 0.1f) continue;

    float dx = player_position.x - position.x;
    float dz = player_position.z - position.z;
    float distance = sqrtf(dx*dx + dz*dz);
    
    if (distance < radius) {
      float ratio = radius / distance;

      player_position.x = shrub.position.x + dx * ratio;
      player_position.z = shrub.position.z + dz * ratio;
    }
  }

  // @todo handle collisions for other entity types
}

void astro::InitGame(Tachyon* tachyon, State& state) {
  MeshLibrary::AddMeshes(tachyon, state);

  // @todo move to ui.cpp
  state.debug_text = Tachyon_CreateUIText("./fonts/CascadiaMonoNF.ttf", 20);
  state.debug_text_large = Tachyon_CreateUIText("./fonts/OpenSans-Regular.ttf", 32);

  Tachyon_InitializeObjects(tachyon);

  ObjectManager::CreateObjects(tachyon, state);
  DataLoader::LoadLevelData(tachyon, state);

  // @todo default this somewhere, or load in from save
  tachyon->scene.camera.position = tVec3f(0.f, 10000.f, 10000.f);
}

void astro::UpdateGame(Tachyon* tachyon, State& state, const float dt) {
  auto& scene = tachyon->scene;

  // Toggle level editor with E
  // @todo dev mode only
  {
    if (did_press_key(tKey::E)) {
      if (state.is_level_editor_open) {
        LevelEditor::CloseLevelEditor(tachyon, state);
      } else {
        LevelEditor::OpenLevelEditor(tachyon, state);
      }
    }
  }

  // @todo dev mode only
  if (state.is_level_editor_open) {
    LevelEditor::HandleLevelEditor(tachyon, state, dt);

    return;
  }

  HandleControls(tachyon, state, dt);
  HandleCollisions(tachyon, state);

  UpdatePlayer(tachyon, state);
  UpdateWaterPlane(tachyon, state);
  UpdateCamera(tachyon, state, dt);
  UpdateAstrolabe(tachyon, state);

  TimeEvolution::HandleAstroTime(tachyon, state, dt);

  // @todo move to ui.cpp
  // @todo debug mode only
  ShowGameStats(tachyon, state);
}