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
#include "astro/player_character.h"
#include "astro/procedural_generation.h"
#include "astro/sfx.h"
#include "astro/spell_system.h"
#include "astro/targeting.h"
#include "astro/time_evolution.h"
#include "astro/ui_system.h"

#define MUSIC_ENABLED 0

using namespace astro;

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
  Tachyon_UseLodByDistance(tachyon, meshes.ground_1, 40000.f);

  // Procedural objects
  Tachyon_UseLodByDistance(tachyon, meshes.grass, 35000.f);
  Tachyon_UseLodByDistance(tachyon, meshes.ground_flower, 35000.f);
  Tachyon_UseLodByDistance(tachyon, meshes.tiny_ground_flower, 35000.f);

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
  Tachyon_ShowHighestLevelsOfDetail(tachyon, meshes.tiny_ground_flower);
}

static void HandleFog(Tachyon* tachyon, State& state) {
  auto& fx = tachyon->fx;

  // @temporary
  // @todo lerp based on day/night alpha
  fx.fog_visibility = state.is_nighttime ? 15000.f : 6000.f;
}

// @todo 3d positioned sfx
static void HandleWalkSounds(Tachyon* tachyon, State& state) {
  float distance_threshold = is_key_held(tKey::CONTROLLER_A) ? 2200.f : 1400.f;
  float last_sound_distance = state.movement_distance - state.last_walk_sound_movement_distance;

  if (last_sound_distance > distance_threshold) {
    auto cycle = state.walk_cycle++;

    if (cycle == 0) {
      Sfx::PlaySound(SFX_GROUND_WALK_1, 0.1f);
    }
    else if (cycle == 1) {
      Sfx::PlaySound(SFX_GROUND_WALK_2, 0.1f);
    }
    else if (cycle == 2) {
      Sfx::PlaySound(SFX_GROUND_WALK_3, 0.1f);

      state.walk_cycle = 0;
    }

    state.last_walk_sound_movement_distance = state.movement_distance;
  }
}

// @todo move to Targeting::
static bool IsInStealthMode(State& state) {
  const float stealth_distance_limit = 8000.f;

  float closest_target_distance = 10000.f;

  for (auto& record : state.targetable_entities) {
    auto* entity = EntityManager::FindEntity(state, record);
    float player_distance = tVec3f::distance(state.player_position, entity->visible_position);

    if (player_distance < closest_target_distance) {
      closest_target_distance = player_distance;
    }

    if (
      player_distance < stealth_distance_limit &&
      entity->enemy_state.mood != ENEMY_IDLE
    ) {
      return false;
    }
  }

  return closest_target_distance < stealth_distance_limit;
}

// @incomplete
static Sound GetCurrentAmbientSound(State& state) {
  if (state.is_nighttime) {
    return SFX_FOREST_NIGHT;
  } else {
    return SFX_FOREST;
  }
}

static void HandleMusicLevels(Tachyon* tachyon, State& state) {
  #if MUSIC_ENABLED == 1

    // Ambient sounds
    {
      Sound current_ambient_sound = GetCurrentAmbientSound(state);

      Sfx::LoopSound(current_ambient_sound, 0.5f);

      if (state.astro_turn_speed != 0.f) {
        Sfx::FadeSoundVolumeTo(current_ambient_sound, 0.f, 500);
      } else {
        Sfx::FadeSoundVolumeTo(current_ambient_sound, 0.5f, 500);
      }
    }

    if (state.bgm_start_time == -1.f) return;

    // Background music
    {
      if (IsInStealthMode(state) || Targeting::IsInCombatMode(state)) {
        BGM::FadeCurrentMusicVolumeTo(0.1f, 500);
      }
      else if (state.astro_turn_speed != 0.f) {
        BGM::FadeCurrentMusicVolumeTo(0.f, 500);
      }
      else {
        BGM::FadeCurrentMusicVolumeTo(0.4f, 2000);
      }
    }

  #endif
}

static void HandleCurrentAreaMusic(Tachyon* tachyon, State& state) {
  #if MUSIC_ENABLED == 1

    if (state.bgm_start_time == -1.f) return;

    // @temporary
    tVec3f village_position = tVec3f(157000.f, 0, -44000.f);

    if (tVec3f::distance(state.player_position, village_position) < 40000.f) {
      BGM::LoopMusic(VILLAGE_1, 0.4f);
    } else {
      BGM::LoopMusic(DIVINATION_WOODREALM, 0.4f);
    }

  #endif
}

static void ShowGameStats(Tachyon* tachyon, State& state) {
  float player_speed = state.player_velocity.magnitude();

  std::string stat_messages[] = {
    "Player " + state.player_position.toString(),
    "HP " + std::to_string(state.player_hp),
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
  EntityManager::CreateEntityAssociations(state);

  // @todo default/load from save
  state.player_position = tVec3f(-13800.f, 0, -5900.f);
  state.player_facing_direction = tVec3f(0, 0, 1.f);
  state.camera_shift = tVec3f(0, 0, 1875.f);

  state.player_light_id = create_point_light();
  state.astrolabe_light_id = create_point_light();

  // @todo default/load from save
  tachyon->scene.camera.position = tVec3f(-13800.f, 10000.f, 2975.f);

  tachyon->scene.scene_time = 0.f;
}

void astro::UpdateGame(Tachyon* tachyon, State& state, const float dt) {
  profile("UpdateGame()");

  auto& scene = tachyon->scene;

  // @temporary
  scene.scene_time += dt;

  // @todo HandleFrameStart()
  {
    state.spells.did_cast_stun_this_frame = false;
    state.dt = dt;
  }

  // Reset fx
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

    LevelEditor::HandleLevelEditor(tachyon, state);

    return;
  }

  // Dev hotkeys
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

      // @todo reset gates
    }

    if (did_press_key(tKey::I)) {
      Items::CollectItem(tachyon, state, ITEM_STUN_SPELL);
      Items::CollectItem(tachyon, state, ITEM_HOMING_SPELL);
      Items::CollectItem(tachyon, state, ASTROLABE_LOWER_LEFT);
      Items::CollectItem(tachyon, state, GATE_KEY);
    }
  }

  #if MUSIC_ENABLED == 1

    if (state.astro_time < 0.f && state.astro_turn_speed == 0.f && state.bgm_start_time == -1.f) {
      BGM::LoopMusic(DIVINATION_WOODREALM, 0.4f);

      state.bgm_start_time = get_scene_time();
    }

  #endif

  Targeting::HandleTargets(tachyon, state);
  ControlSystem::HandleControls(tachyon, state);
  CollisionSystem::HandleCollisions(tachyon, state);
  SpellSystem::HandleSpells(tachyon, state);
  Items::HandleItemPickup(tachyon, state);
  UISystem::HandleDialogue(tachyon, state);
  HandleFog(tachyon, state);
  HandleWalkSounds(tachyon, state);
  HandleCurrentAreaMusic(tachyon, state);
  HandleMusicLevels(tachyon, state);

  TimeEvolution::UpdateAstroTime(tachyon, state);
  ProceduralGeneration::UpdateProceduralObjects(tachyon, state);
  CameraSystem::UpdateCamera(tachyon, state);
  Astrolabe::Update(tachyon, state);
  PlayerCharacter::UpdatePlayer(tachyon, state);
  UpdateWaterPlane(tachyon, state);
  UpdateLevelsOfDetail(tachyon, state);

  // @todo HandleFrameEnd()
  {
    auto& fx = tachyon->fx;
    float max_blur_factor = 0.98f - 5.f * state.dt;

    state.movement_distance += tVec3f::distance(state.player_position, state.last_player_position);
    state.last_player_position = state.player_position;

    fx.accumulation_blur_factor = sqrtf(abs(state.astro_turn_speed)) * 4.f;

    if (fx.accumulation_blur_factor > max_blur_factor) {
      fx.accumulation_blur_factor = max_blur_factor;
    }

    // Time warp effects
    {
      state.time_warp_start_radius = Tachyon_Lerpf(state.time_warp_start_radius, 30000.f, state.dt);
      state.time_warp_end_radius = Tachyon_Lerpf(state.time_warp_end_radius, 30000.f, state.dt);

      fx.player_position = state.player_position;
      fx.astro_time_warp = state.astro_turn_speed / Astrolabe::GetMaxTurnSpeed();

      fx.astro_time_warp_start_radius = state.time_warp_start_radius;
      fx.astro_time_warp_end_radius = state.time_warp_end_radius;
    }

    // Vignette effects in stealth mode
    {
      float desired_vignette_intensity = IsInStealthMode(state) ? 1.f : 0.f;

      fx.vignette_intensity = Tachyon_Lerpf(fx.vignette_intensity, desired_vignette_intensity, 2.f * state.dt);
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