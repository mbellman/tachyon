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
  // Update position
  {
    state.player_position += state.player_velocity * 5.f * dt;
  }

  // Update facing direction
  {
    tVec3f desired_facing_direction = state.player_facing_direction;
    float turning_speed = 5.f;

    if (state.has_target && !state.is_escaping_target) {
      // When we're focused on a target, face it and turn much more quickly
      auto& target = *EntityManager::FindEntity(state, state.target_entity);

      desired_facing_direction = (target.visible_position - state.player_position).unit();
    }
    else if (state.player_velocity.magnitude() > 0.01f) {
      // Without a target, use our velocity vector to influence facing direction
      desired_facing_direction = state.player_velocity.unit();
    }

    // When astro turning, don't change our facing direction at all,
    // since targeted entities may jitter and jump about rapidly,
    // and we don't want the facing direction being thrown off
    if (abs(state.astro_turn_speed) > 0.05f) {
      turning_speed = 0.f;
    }

    state.player_facing_direction = tVec3f::lerp(state.player_facing_direction, desired_facing_direction, turning_speed * dt).unit();
  }

  // Update model
  {
    auto& player = objects(state.meshes.player)[0]; 

    player.position = state.player_position;
    // @temporary
    player.scale = tVec3f(600.f, 1500.f, 600.f);
    player.color = tVec3f(0, 0.2f, 1.f);

    player.rotation = Quaternion::FromDirection(state.player_facing_direction, tVec3f(0, 1.f, 0));

    commit(player);
  }
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
    "Astro time: " + std::to_string(state.astro_time),
    "Astro turn speed: " + std::to_string(state.astro_turn_speed)
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
  base.material = tVec4f(0.1f, 1.f, 0, 0.2f);

  ring.color = tVec3f(0.2f, 0.4f, 1.f);
  ring.material = tVec4f(0.2f, 1.f, 0, 0);

  hand.color = tVec3f(1.f, 0.6f, 0.2f);
  hand.material = tVec4f(0.1f, 1.f, 0, 0.6f);

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

// @todo targeting.cpp
static void StoreClosestEnemy(Tachyon* tachyon, State& state, EntityRecord& record) {
  const float target_distance_limit = 10000.f;
  float closest_distance = target_distance_limit;
  EntityRecord candidate;

  // @todo refactor
  for_entities(state.low_guards) {
    auto& entity = state.low_guards[i];
    float distance = (state.player_position - entity.visible_position).magnitude();

    if (distance < closest_distance && entity.visible_scale.x != 0.f) {
      closest_distance = distance;

      candidate.id = entity.id;
      candidate.type = entity.type;
    }
  }

  // @todo refactor
  for_entities(state.bandits) {
    auto& entity = state.bandits[i];
    float distance = (state.player_position - entity.visible_position).magnitude();

    if (distance < closest_distance && entity.visible_scale.x != 0.f) {
      closest_distance = distance;

      candidate.id = entity.id;
      candidate.type = entity.type;
    }
  }

  if (closest_distance == target_distance_limit) {
    state.has_target = false;

    record.id = -1;
    record.type = UNSPECIFIED;
  }
  else if (!state.has_target) {
    state.has_target = true;
    state.target_start_time = tachyon->running_time;

    record = candidate;

    if (is_key_held(tKey::CONTROLLER_A)) {
      state.is_escaping_target = true;
    }
  }
}

// @todo move to ui_system.cpp
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
  } else {
    state.dialogue_message = "";
    state.dialogue_start_time = 0.f;
  }
}

static void UpdateLevelsOfDetail(Tachyon* tachyon, State& state) {
  profile("UpdateLevelsOfDetail()");

  auto& meshes = state.meshes;

  // Decorative objects
  Tachyon_UseLodByDistance(tachyon, meshes.rock_1, 35000.f);
  Tachyon_UseLodByDistance(tachyon, meshes.ground_1, 35000.f);

  // Procedural objects
  Tachyon_UseLodByDistance(tachyon, meshes.grass, 35000.f);
  Tachyon_UseLodByDistance(tachyon, meshes.small_grass, 35000.f);
}

static void ShowHighestLevelsOfDetail(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  Tachyon_ShowHighestLevelsOfDetail(tachyon, meshes.rock_1);
  Tachyon_ShowHighestLevelsOfDetail(tachyon, meshes.ground_1);
  Tachyon_ShowHighestLevelsOfDetail(tachyon, meshes.grass);
  Tachyon_ShowHighestLevelsOfDetail(tachyon, meshes.small_grass);
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

  tachyon->scene.scene_time = 0.f;
}

void astro::UpdateGame(Tachyon* tachyon, State& state, const float dt) {
  profile("UpdateGame()");

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
    ShowHighestLevelsOfDetail(tachyon, state);

    LevelEditor::HandleLevelEditor(tachyon, state, dt);

    return;
  }

  ControlSystem::HandleControls(tachyon, state, dt);
  CollisionSystem::HandleCollisions(tachyon, state);
  SpellSystem::HandleSpells(tachyon, state, dt);
  HandleDialogue(tachyon, state);

  // @todo targeting.cpp
  {
    StoreClosestEnemy(tachyon, state, state.target_entity);

    auto& reticle = objects(state.meshes.target_reticle)[0];

    if (state.has_target) {
      auto& entity = *EntityManager::FindEntity(state, state.target_entity);

      reticle.position = entity.visible_position;
      reticle.position.y += entity.visible_scale.y + 800.f;

      reticle.scale = tVec3f(300.f);
      reticle.color = tVec4f(1.f, 0.8f, 0.2f, 0.4f);
      reticle.rotation = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), 2.f * tachyon->running_time);
    } else {
      reticle.scale = tVec3f(0.f);
    }

    commit(reticle);
  }

  TimeEvolution::UpdateAstroTime(tachyon, state, dt);
  ProceduralGeneration::UpdateProceduralObjects(tachyon, state);
  CameraSystem::UpdateCamera(tachyon, state, dt);
  UpdatePlayer(tachyon, state, dt);
  UpdateWaterPlane(tachyon, state);
  UpdateAstrolabe(tachyon, state);
  UpdateLevelsOfDetail(tachyon, state);

  // @todo HandleFrameEnd()
  {
    auto& fx = tachyon->fx;
    float max_blur_factor = 0.98f - 5.f * dt;

    state.last_player_position = state.player_position;

    fx.accumulation_blur_factor = sqrtf(abs(state.astro_turn_speed)) * 4.f;

    if (fx.accumulation_blur_factor > max_blur_factor) {
      fx.accumulation_blur_factor = max_blur_factor;
    }

    tachyon->scene.scene_time += dt;

    // @todo ui.cpp
    // @todo debug mode only
    ShowGameStats(tachyon, state);
  }
}