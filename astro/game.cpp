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
#include "astro/ui_system.h"

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
  water_plane.scale = tVec3f(400000.f, 1.f, 400000.f);
  water_plane.color = tVec3f(0, 0.1f, 0.3f);
  water_plane.material = tVec4f(0.1f, 1.f, 0, 0.5f);

  water_plane.position.y = -1800.f + 22.f * state.astro_time;

  if (water_plane.position.y > -1800.f) water_plane.position.y = -1800.f;
  if (water_plane.position.y < -3500.f) water_plane.position.y = -3500.f;

  commit(water_plane);

  state.water_level = water_plane.position.y;
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
  // @todo handle distance LoD stuff in entity behavior files;
  // it's annoying to manage them here
  Tachyon_UseLodByDistance(tachyon, meshes.shrub_leaves, 35000.f);
  Tachyon_UseLodByDistance(tachyon, meshes.oak_tree_roots, 40000.f);
  Tachyon_UseLodByDistance(tachyon, meshes.oak_tree_trunk, 40000.f);
  Tachyon_UseLodByDistance(tachyon, meshes.oak_tree_branches, 40000.f);
  Tachyon_UseLodByDistance(tachyon, meshes.oak_tree_leaves, 40000.f);
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
  ProceduralGeneration::RebuildAllProceduralObjects(tachyon, state);

  // Perform entity associations
  // @todo factor
  // @todo redo this upon leaving the editor
  for_all_entity_types() {
    for_entities_of_type(type) {
      auto& entity = entities[i];

      if (entity.associated_entity_name != "") {
        GameEntity* associated_entity = EntityManager::FindEntityByUniqueName(state, entity.associated_entity_name);

        if (associated_entity != nullptr) {
          entity.associated_entity_record.type = associated_entity->type;
          entity.associated_entity_record.id = associated_entity->id;

          // @todo return in the eventual refactored function
        }
      }
    }
  }

  // @todo default/load from save
  state.player_position = tVec3f(-13800.f, 0, -5900.f);
  state.player_facing_direction = tVec3f(0, 0, 1.f);
  state.camera_shift = tVec3f(0, 0, 1875.f);

  // @todo default/load from save
  tachyon->scene.camera.position = tVec3f(-13800.f, 10000.f, 2975.f);

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
  scene.scene_time += dt;

  // Reset astro turn fx
  // @todo move to editor
  {
    auto& fx = tachyon->fx;

    fx.astro_time_warp_start_radius = 0.f;
    fx.astro_time_warp_end_radius = 0.f;
  }

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

  // Special dev hotkeys
  // @todo dev mode only
  {
    if (did_press_key(tKey::SPACE)) {
      state.show_game_stats = !state.show_game_stats;
    }

    if (did_press_key(tKey::ARROW_DOWN)) {
      state.use_zoomed_out_camera = !state.use_zoomed_out_camera;
    }

    if (did_press_key(tKey::R)) {
      for_entities(state.light_posts) {
        auto& entity = state.light_posts[i];

        entity.did_activate = false;
        entity.astro_activation_time = 0.f;
        entity.game_activation_time = -1.f;
        entity.is_astro_synced = false;
      }
    }
  }

  // @todo HandleFrameStart()
  {
    state.spells.did_cast_stun_this_frame = false;
  }

  Targeting::HandleTargets(tachyon, state);
  ControlSystem::HandleControls(tachyon, state, dt);
  CollisionSystem::HandleCollisions(tachyon, state, dt);
  SpellSystem::HandleSpells(tachyon, state, dt);
  Items::HandleItemPickup(tachyon, state);
  UISystem::HandleDialogue(tachyon, state);
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
      fx.astro_time_warp = state.astro_turn_speed / Astrolabe::GetMaxTurnSpeed();

      fx.astro_time_warp_start_radius = state.time_warp_start_radius;
      fx.astro_time_warp_end_radius = state.time_warp_end_radius;
    }

    auto& velocity = state.player_velocity;
    float speed = velocity.magnitude();
    tVec3f foliage_movement_offset = (speed > 0.f ? velocity.invert().unit() * 500.f : 0.f);

    scene.foliage_mover_position = state.player_position + foliage_movement_offset;
    scene.foliage_mover_velocity = velocity;

    if (speed > 300.f) {
      scene.foliage_mover_velocity = velocity.unit() * 300.f;
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