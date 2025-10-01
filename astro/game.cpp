#include "astro/game.h"
#include "astro/camera_system.h"
#include "astro/collision_system.h"
#include "astro/control_system.h"
#include "astro/data_loader.h"
#include "astro/entity_dispatcher.h"
#include "astro/entity_manager.h"
#include "astro/level_editor.h"
#include "astro/mesh_library.h"
#include "astro/object_manager.h"
#include "astro/procedural_generation.h"
#include "astro/spell_system.h"
#include "astro/time_evolution.h"

using namespace astro;

static void UpdatePlayer(Tachyon* tachyon, State& state, const float dt) {
  state.player_position += state.player_velocity * 5.f * dt;

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
  water_plane.scale = tVec3f(100000.f, 1.f, 100000.f);
  water_plane.color = tVec3f(0, 0.1f, 0.3f);
  water_plane.material = tVec4f(0.1f, 1.f, 0, 0.5f);

  water_plane.position.y = -1800.f + 22.f * state.astro_time;

  if (water_plane.position.y > -1800.f) water_plane.position.y = -1800.f;
  if (water_plane.position.y < -4000.f) water_plane.position.y = -4000.f;

  commit(water_plane);

  state.water_level = water_plane.position.y;
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

// @todo target_system.cpp
static void StoreClosestEnemy(Tachyon* tachyon, State& state, EntityRecord& record) {
  float target_distance_limit = state.has_target ? 14000.f : 8000.f;
  float closest_distance = target_distance_limit;

  record.id = -1;
  record.type = UNSPECIFIED;

  // @todo refactor
  for_entities(state.low_guards) {
    auto& entity = state.low_guards[i];
    float distance = (state.player_position - entity.visible_position).magnitude();

    if (distance < closest_distance) {
      closest_distance = distance;

      record.id = entity.id;
      record.type = entity.type;
    }
  }

  // @todo refactor
  for_entities(state.bandits) {
    auto& entity = state.bandits[i];
    float distance = (state.player_position - entity.visible_position).magnitude();

    if (distance < closest_distance) {
      closest_distance = distance;

      record.id = entity.id;
      record.type = entity.type;
    }
  }

  if (!state.has_target && closest_distance < target_distance_limit) {
    state.has_target = true;
    state.target_start_time = tachyon->running_time;
  }
  else if (closest_distance == target_distance_limit) {
    state.has_target = false;
  }
}

// @todo dialogue_system.cpp
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

void astro::InitGame(Tachyon* tachyon, State& state) {
  MeshLibrary::AddMeshes(tachyon, state);

  // @todo ui.cpp
  state.debug_text = Tachyon_CreateUIText("./fonts/CascadiaMonoNF.ttf", 19);
  state.debug_text_large = Tachyon_CreateUIText("./fonts/OpenSans-Regular.ttf", 32);

  Tachyon_InitializeObjects(tachyon);

  ObjectManager::CreateObjects(tachyon, state);
  DataLoader::LoadLevelData(tachyon, state);
  ProceduralGeneration::RebuildProceduralObjects(tachyon, state);

  // @todo default this somewhere, or load in from save
  tachyon->scene.camera.position = tVec3f(0.f, 10000.f, 10000.f);
}

void astro::UpdateGame(Tachyon* tachyon, State& state, const float dt) {
  profiler_start("UpdateGame()");

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

  ControlSystem::HandleControls(tachyon, state, dt);
  CollisionSystem::HandleCollisions(tachyon, state);
  SpellSystem::HandleSpells(tachyon, state, dt);
  HandleDialogue(tachyon, state);

  // @todo target_system.cpp
  StoreClosestEnemy(tachyon, state, state.target_entity);

  TimeEvolution::UpdateAstroTime(tachyon, state, dt);
  ProceduralGeneration::UpdateProceduralObjects(tachyon, state);
  CameraSystem::UpdateCamera(tachyon, state, dt);
  UpdatePlayer(tachyon, state, dt);
  UpdateWaterPlane(tachyon, state);
  UpdateAstrolabe(tachyon, state);

  // @todo HandleFrameEnd()
  {
    auto& fx = tachyon->fx;

    state.last_player_position = state.player_position;

    fx.accumulation_blur_factor = sqrtf(abs(state.astro_turn_speed * 4.f));

    if (fx.accumulation_blur_factor > 0.95f) {
      fx.accumulation_blur_factor = 0.95f;
    }

    // @todo ui.cpp
    // @todo debug mode only
    ShowGameStats(tachyon, state);
  }


  profiler_end();
}