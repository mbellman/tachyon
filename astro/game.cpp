#include "astro/game.h"
#include "astro/astrolabe.h"
#include "astro/bgm.h"
#include "astro/camera_system.h"
#include "astro/collision_system.h"
#include "astro/control_system.h"
#include "astro/data_loader.h"
#include "astro/entity_dispatcher.h"
#include "astro/entity_manager.h"
#include "astro/items.h"
#include "astro/level_editor.h"
#include "astro/mesh_library.h"
#include "astro/object_manager.h"
#include "astro/procedural_generation.h"
#include "astro/sfx.h"
#include "astro/spell_system.h"
#include "astro/targeting.h"
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

    if (state.has_target) {
      // When we're focused on a target, face it and turn much more quickly
      auto& target = *EntityManager::FindEntity(state, state.target_entity);

      desired_facing_direction = (target.visible_position - state.player_position).xz().unit();
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
  water_plane.scale = tVec3f(200000.f, 1.f, 200000.f);
  water_plane.color = tVec3f(0, 0.1f, 0.3f);
  water_plane.material = tVec4f(0.1f, 1.f, 0, 0.5f);

  water_plane.position.y = -1800.f + 22.f * state.astro_time;

  if (water_plane.position.y > -1800.f) water_plane.position.y = -1800.f;
  if (water_plane.position.y < -4000.f) water_plane.position.y = -4000.f;

  commit(water_plane);

  state.water_level = water_plane.position.y;
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
  Tachyon_UseLodByDistance(tachyon, meshes.ground_flower, 35000.f);

  // Entity parts
  Tachyon_UseLodByDistance(tachyon, meshes.oak_tree_roots, 40000.f);
  Tachyon_UseLodByDistance(tachyon, meshes.oak_tree_trunk, 40000.f);
  Tachyon_UseLodByDistance(tachyon, meshes.oak_tree_branches, 40000.f);
  Tachyon_UseLodByDistance(tachyon, meshes.flower_bush_leaves, 35000.f);
}

static void ShowHighestLevelsOfDetail(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  Tachyon_ShowHighestLevelsOfDetail(tachyon, meshes.rock_1);
  Tachyon_ShowHighestLevelsOfDetail(tachyon, meshes.ground_1);
  Tachyon_ShowHighestLevelsOfDetail(tachyon, meshes.grass);
  Tachyon_ShowHighestLevelsOfDetail(tachyon, meshes.ground_flower);
}

// @todo 3d positioned sfx
static void HandleWalkSounds(Tachyon* tachyon, State& state) {
  float distance_threshold = is_key_held(tKey::CONTROLLER_A) ? 2200.f : 1400.f;
  float last_sound_distance = state.movement_distance - state.last_walk_sound_movement_distance;

  if (last_sound_distance > distance_threshold) {
    auto cycle = state.walk_cycle++;

    if (cycle == 0) {
      Sfx::PlaySound(SFX_GROUND_WALK_1, 0.3f);
    }
    else if (cycle == 1) {
      Sfx::PlaySound(SFX_GROUND_WALK_2, 0.3f);
    }
    else if (cycle == 2) {
      Sfx::PlaySound(SFX_GROUND_WALK_3, 0.3f);

      state.walk_cycle = 0;
    }

    state.last_walk_sound_movement_distance = state.movement_distance;
  }
}

static void ShowGameStats(Tachyon* tachyon, State& state) {
  float player_speed = state.player_velocity.magnitude();

  std::string stat_messages[] = {
    "Player " + state.player_position.toString(),
    "Speed " + std::to_string(player_speed),
    "Camera " + tachyon->scene.camera.position.toString(),
    "Astro time: " + std::to_string(state.astro_time),
    "Astro turn speed: " + std::to_string(state.astro_turn_speed),
    "Speaking entity: " + std::to_string(state.speaking_entity_record.id)
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

void astro::InitGame(Tachyon* tachyon, State& state) {
  MeshLibrary::AddMeshes(tachyon, state);

  // @todo ui.cpp
  state.debug_text = Tachyon_CreateUIText("./fonts/CascadiaMonoNF.ttf", 19);
  state.debug_text_large = Tachyon_CreateUIText("./fonts/OpenSans-Regular.ttf", 32);

  Tachyon_InitializeObjects(tachyon);

  ObjectManager::CreateObjects(tachyon, state);
  DataLoader::LoadLevelData(tachyon, state);
  Items::SpawnItemObjects(tachyon, state);
  ProceduralGeneration::RebuildProceduralObjects(tachyon, state);

  // @todo default this somewhere, or load in from save
  tachyon->scene.camera.position = tVec3f(0.f, 10000.f, 10000.f);

  tachyon->scene.scene_time = 0.f;

  // @todo configure music per area
  {
    // BGM::LoopMusic(DIVINATION_WOODREALM);
  }
}

void astro::UpdateGame(Tachyon* tachyon, State& state, const float dt) {
  profile("UpdateGame()");

  auto& scene = tachyon->scene;

  // @temporary
  tachyon->scene.scene_time += dt;

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

  if (did_press_key(tKey::SPACE)) {
    state.show_game_stats = !state.show_game_stats;
  }

  Targeting::HandleTargets(tachyon, state);
  ControlSystem::HandleControls(tachyon, state, dt);
  CollisionSystem::HandleCollisions(tachyon, state, dt);
  SpellSystem::HandleSpells(tachyon, state, dt);
  Items::HandleItemPickup(tachyon, state);
  HandleDialogue(tachyon, state);
  HandleWalkSounds(tachyon, state);

  TimeEvolution::UpdateAstroTime(tachyon, state, dt);
  ProceduralGeneration::UpdateProceduralObjects(tachyon, state);
  CameraSystem::UpdateCamera(tachyon, state, dt);
  Astrolabe::Update(tachyon, state);
  UpdatePlayer(tachyon, state, dt);
  UpdateWaterPlane(tachyon, state);
  UpdateLevelsOfDetail(tachyon, state);

  // @todo HandleFrameEnd()
  {
    auto& fx = tachyon->fx;
    float max_blur_factor = 0.98f - 5.f * dt;

    state.movement_distance += tVec3f::distance(state.player_position, state.last_player_position);
    state.last_player_position = state.player_position;

    fx.accumulation_blur_factor = sqrtf(abs(state.astro_turn_speed)) * 4.f;

    if (fx.accumulation_blur_factor > max_blur_factor) {
      fx.accumulation_blur_factor = max_blur_factor;
    }

    // Time warp effects
    {
      state.time_warp_start_radius = Tachyon_Lerpf(state.time_warp_start_radius, 30000.f, dt);
      state.time_warp_end_radius = Tachyon_Lerpf(state.time_warp_end_radius, 30000.f, dt);

      fx.player_position = state.player_position;
      fx.astro_time_warp = abs(state.astro_turn_speed / 0.25f);

      fx.astro_time_warp_start_radius = state.time_warp_start_radius;
      fx.astro_time_warp_end_radius = state.time_warp_end_radius;
    }

    add_dev_label("Start radius", std::to_string(fx.astro_time_warp_start_radius));
    add_dev_label("End radius", std::to_string(fx.astro_time_warp_end_radius));

    auto& velocity = state.player_velocity;
    float speed = velocity.magnitude();
    tVec3f foliage_movement_offset = (speed > 0.f ? velocity.invert().unit() * 500.f : 0.f);

    tachyon->scene.foliage_mover_position = state.player_position + foliage_movement_offset;
    tachyon->scene.foliage_mover_velocity = velocity;

    if (speed > 300.f) {
      tachyon->scene.foliage_mover_velocity = velocity.unit() * 300.f;
    }

    // @todo ui.cpp
    // @todo debug mode only
    if (state.show_game_stats) {
      ShowGameStats(tachyon, state);
    } else {
      tachyon->dev_labels.clear();
    }
  }
}