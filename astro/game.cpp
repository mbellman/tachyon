#include "astro/game.h"
#include "astro/collision_system.h"
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

  water_plane.position.y = -1800.f + 22.f * state.astro_time;

  if (water_plane.position.y > -1800.f) water_plane.position.y = -1800.f;
  if (water_plane.position.y < -4000.f) water_plane.position.y = -4000.f;

  commit(water_plane);

  state.water_level = water_plane.position.y;
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

  // @todo refactor
  if (room_x != 0 || room_z != 0) {
    float player_speed = state.player_velocity.magnitude();

    if (player_speed > 0.01f) {
      tVec3f unit_velocity = state.player_velocity / player_speed;
      tVec3f camera_bias = tVec3f(0, 0, 0.25f) * abs(unit_velocity.z);
      tVec3f new_camera_shift = (unit_velocity + camera_bias) * 2000.f;

      state.camera_shift = tVec3f::lerp(state.camera_shift, new_camera_shift, 2.f * dt);
    }

    new_camera_position = state.player_position + state.camera_shift;
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

static void ShowDialogue(Tachyon* tachyon, State& state, const std::string& message) {
  if (
    state.dialogue_message == message &&
    tachyon->running_time - state.dialogue_start_time < 6.f
  ) {
    // Don't re-show currently-displayed dialogue
    return;
  }

  state.dialogue_message = message;
  state.dialogue_start_time = tachyon->running_time;
}

static void HandleDialogue(Tachyon* tachyon, State& state) {
  float dialogue_age = tachyon->running_time - state.dialogue_start_time;

  if (dialogue_age < 6.f) {
    float alpha = 1.f;
    if (dialogue_age < 0.2f) alpha = dialogue_age * 5.f;
    if (dialogue_age > 5.f) alpha = 1.f - (dialogue_age - 5.f);

    Tachyon_DrawUIText(tachyon, state.debug_text_large, {
      .screen_x = tachyon->window_width / 2,
      .screen_y = tachyon->window_height - 150,
      .centered = true,
      .alpha = alpha,
      .string = state.dialogue_message
    });
  }
}

// @todo move to its own file
static void HandleControls(Tachyon* tachyon, State& state, const float dt) {
  // Handle movement actions
  // @todo refactor
  {
    if (
      tachyon->left_trigger == 0.f &&
      tachyon->right_trigger == 0.f &&
      abs(state.astro_turn_speed) < 0.1f
    ) {
      if (is_key_held(tKey::ARROW_UP) || is_key_held(tKey::W)) {
        state.player_velocity += tVec3f(0, 0, -1.f) * 10000.f * dt;
      }
  
      if (is_key_held(tKey::ARROW_LEFT) || is_key_held(tKey::A)) {
        state.player_velocity += tVec3f(-1.f, 0, 0) * 10000.f * dt;
      }
  
      if (is_key_held(tKey::ARROW_RIGHT) || is_key_held(tKey::D)) {
        state.player_velocity += tVec3f(1.f, 0, 0) * 10000.f * dt;
      }
  
      if (is_key_held(tKey::ARROW_DOWN) || is_key_held(tKey::S)) {
        state.player_velocity += tVec3f(0, 0, 1.f) * 10000.f * dt;
      }
  
      state.player_velocity.x += tachyon->left_stick.x * 10000.f * dt;
      state.player_velocity.z += tachyon->left_stick.y * 10000.f * dt;
    }

    state.player_velocity *= 1.f - 10.f * dt;
    state.player_position += state.player_velocity * 5.f * dt;
  }

  // Handle astro turn actions
  // @todo refactor
  {
    const float astro_turn_rate = 0.8f;
    const float astro_slowdown_rate = 3.f;

    // @todo increase this once the appropriate item is obtained
    float max_astro_time = 0.f;
    // @todo decrease this once the appropriate item is obtained
    float min_astro_time = -158.f;

    bool started_turning = (
      (state.last_frame_left_trigger == 0.f && tachyon->left_trigger != 0.f) ||
      (state.last_frame_right_trigger == 0.f && tachyon->right_trigger != 0.f)
    );

    // Track the initial time when we start turning
    if (started_turning) {
      state.astro_time_at_start_of_turn = state.astro_time;
    }

    // Handle reverse/forward turn actions
    state.astro_turn_speed -= tachyon->left_trigger * astro_turn_rate * dt;
    state.astro_turn_speed += tachyon->right_trigger * astro_turn_rate * dt;

    // Prevent time changes past max time
    if (state.astro_time > max_astro_time && state.astro_turn_speed > 0.f) {
      state.astro_turn_speed = 0.f;

      if (state.astro_time_at_start_of_turn >= 0.f) {
        ShowDialogue(tachyon, state, "The astrolabe's mechanism resists.");
      }
    }

    // Prevent time changes before min time
    if (state.astro_time < min_astro_time && state.astro_turn_speed < 0.f) {
      state.astro_turn_speed = 0.f;

      if (state.astro_time_at_start_of_turn <= min_astro_time + 1.f) {
        ShowDialogue(tachyon, state, "The astrolabe's mechanism resists.");
      }
    }

    // Slow down toward max time
    if (state.astro_turn_speed > 0.f) {
      float slowdown_threshold = max_astro_time - (max_astro_time - state.astro_time_at_start_of_turn) * 0.1f;

      if (slowdown_threshold < max_astro_time - 5.f) {
        // Enforce a limit on how far away the slowdown threshold is
        // from the stopping value
        slowdown_threshold = max_astro_time - 5.f;
      }

      if (state.astro_time > slowdown_threshold) {
        float threshold_distance = abs(slowdown_threshold - state.astro_time);
        float threshold_to_limit = max_astro_time - slowdown_threshold;
        float slowdown_factor = 40.f * powf(threshold_distance / threshold_to_limit, 2.f);

        state.astro_turn_speed *= 1.f - slowdown_factor * dt;

        ShowDialogue(tachyon, state, "The astrolabe stopped turning.");
      }
    }

    // Slow down toward min time
    if (state.astro_turn_speed < 0.f) {
      float slowdown_threshold = min_astro_time + abs(state.astro_time_at_start_of_turn - min_astro_time) * 0.1f;

      if (slowdown_threshold > min_astro_time + 5.f) {
        // Enforce a limit on how far away the slowdown threshold is
        // from the stopping value
        slowdown_threshold = min_astro_time + 5.f;
      }

      if (state.astro_time < slowdown_threshold) {
        float threshold_distance = abs(slowdown_threshold - state.astro_time);
        float threshold_to_limit = abs(min_astro_time - slowdown_threshold);
        float slowdown_factor = 40.f * powf(threshold_distance / threshold_to_limit, 2.f);

        state.astro_turn_speed *= 1.f - slowdown_factor * dt;

        if (state.astro_time_at_start_of_turn > min_astro_time + 1.f) {
          ShowDialogue(tachyon, state, "The astrolabe stopped turning.");
        }
      }
    }

    state.astro_time += state.astro_turn_speed * 100.f * dt;

    // Reduce turn rate gradually
    state.astro_turn_speed *= 1.f - astro_slowdown_rate * dt;

    // Stop turning at a low enough speed
    if (abs(state.astro_turn_speed) < 0.001f) {
      state.astro_turn_speed = 0.f;
    }

    state.last_frame_left_trigger = tachyon->left_trigger;
    state.last_frame_right_trigger = tachyon->right_trigger;
  }
}

void astro::InitGame(Tachyon* tachyon, State& state) {
  MeshLibrary::AddMeshes(tachyon, state);

  // @todo move to ui.cpp
  state.debug_text = Tachyon_CreateUIText("./fonts/CascadiaMonoNF.ttf", 19);
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
  HandleDialogue(tachyon, state);

  CollisionSystem::HandleCollisions(tachyon, state);

  UpdatePlayer(tachyon, state);
  UpdateWaterPlane(tachyon, state);
  UpdateCamera(tachyon, state, dt);
  UpdateAstrolabe(tachyon, state);

  TimeEvolution::HandleAstroTime(tachyon, state, dt);

  // @todo move to ui.cpp
  // @todo debug mode only
  ShowGameStats(tachyon, state);
}