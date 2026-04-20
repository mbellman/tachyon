#include "astro/game.h"
#include "astro/animated_entities.h"
#include "astro/astrolabe.h"
#include "astro/bgm.h"
#include "astro/camera_system.h"
#include "astro/collision_system.h"
#include "astro/control_system.h"
#include "astro/data_loader.h"
#include "astro/dynamic_fauna.h"
#include "astro/entity_behaviors/behavior.h"
#include "astro/entity_dispatcher.h"
#include "astro/entity_manager.h"
#include "astro/environment.h"
#include "astro/facade_geometry.h"
#include "astro/game_events.h"
#include "astro/items.h"
#include "astro/level_editor.h"
#include "astro/mesh_library.h"
#include "astro/particles.h"
#include "astro/player_character.h"
#include "astro/procedural_generation.h"
#include "astro/procedural_growth.h"
#include "astro/sfx.h"
#include "astro/spell_system.h"
#include "astro/targeting.h"
#include "astro/time_evolution.h"
#include "astro/ui_system.h"

// @todo dev_tools.cpp
#include "astro/entity_behaviors/SmallBird.h"

using namespace astro;

static void CreateConstantObjects(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  create(meshes.player_head);
  create(meshes.player_wand);
  create(meshes.player_lantern);

  create(meshes.water_plane);

  // Environment meshes
  {
    for_range(1, 100) {
      create(meshes.snow_particle);
    }

    for_range(1, 30) {
      create(meshes.stray_leaf);
    }

    for_range(1, 1000) {
      create(meshes.dust_mote);
    }
  }

  for (uint16 i = 0; i < 200; i++) {
    create(meshes.tree_mushroom);
  }

  for (uint16 i = 0; i < 1000; i++) {
    create(meshes.vine_leaf);
  }

  for (uint16 i = 0; i < 300; i++) {
    create(meshes.vine_flower);
  }

  for_range(1, 1000) {
    create(meshes.tree_flower);
  }

  // Clothing + armor
  for_range(1, 10) {
    create(meshes.lesser_helmet);
  }

  create(meshes.astrolabe_rear);
  create(meshes.astrolabe_base);
  create(meshes.astrolabe_plate);
  create(meshes.astrolabe_plate2);
  create(meshes.astrolabe_plate3);
  create(meshes.astrolabe_plate4);
  create(meshes.astrolabe_ring);
  create(meshes.astrolabe_hand);

  create(meshes.item_gate_key);

  create(meshes.target_reticle);

  // @todo dev mode only
  {
    for (uint16 i = 0; i < 100; i++) {
      create(meshes.debug_skeleton_bone);
    }
  }
}

static void UpdateWaterPlane(Tachyon* tachyon, State& state) {
  auto& water_plane = objects(state.meshes.water_plane)[0];

  water_plane.position = tVec3f(0, -3000.f, 0);
  water_plane.scale = tVec3f(400000.f, 1.f, 400000.f);
  water_plane.color = tVec3f(0, 0.1f, 0.3f);
  water_plane.material = tVec4f(0.1f, 1.f, 0, 0.5f);

  commit(water_plane);

  state.water_level = water_plane.position.y;
}

static void UpdateLevelsOfDetail(Tachyon* tachyon, State& state) {
  profile("UpdateLevelsOfDetail()");

  auto& meshes = state.meshes;

  // @temporary @todo @optimize improve culling behavior
  static int frame = 0;

  frame++;

  // Decorative objects
  if (frame == 0 || frame % 3 == 0) {
    Tachyon_UseLodByDistance(tachyon, meshes.rock_1, 50000.f);
    Tachyon_UseLodByDistance(tachyon, meshes.rock_2, 50000.f);
    Tachyon_UseLodByDistance(tachyon, meshes.river_edge, 50000.f);
    Tachyon_UseLodByDistance(tachyon, meshes.ground_1, 50000.f);
    Tachyon_UseLodByDistance(tachyon, meshes.lookout_tower, 60000.f);
  }

  // Procedural objects
  if (frame == 0 || frame % 3 == 1) Tachyon_UseLodByDistance(tachyon, meshes.ground_flower, 35000.f);
  if (frame == 0 || frame % 3 == 2) Tachyon_UseLodByDistance(tachyon, meshes.tiny_ground_flower, 35000.f);
}

static void ShowHighestLevelsOfDetail(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  Tachyon_ShowHighestLevelsOfDetail(tachyon, meshes.rock_1);
  Tachyon_ShowHighestLevelsOfDetail(tachyon, meshes.rock_2);
  Tachyon_ShowHighestLevelsOfDetail(tachyon, meshes.river_edge);
  Tachyon_ShowHighestLevelsOfDetail(tachyon, meshes.ground_1);
  Tachyon_ShowHighestLevelsOfDetail(tachyon, meshes.grass);
  Tachyon_ShowHighestLevelsOfDetail(tachyon, meshes.ground_flower);
  Tachyon_ShowHighestLevelsOfDetail(tachyon, meshes.tiny_ground_flower);
  Tachyon_ShowHighestLevelsOfDetail(tachyon, meshes.lookout_tower);
}

// @todo dev_tools.cpp
static void HandleInGameDevHotkeys(Tachyon* tachyon, State& state) {
  // Toggling game stats
  {
    if (did_press_key(tKey::SPACE)) {
      state.show_game_stats = !state.show_game_stats;
      tachyon->scene.use_close_camera_disocclusion = !state.show_game_stats;
    }
  }

  // Toggling camera zoom
  {
    if (did_press_key(tKey::ARROW_DOWN)) {
      state.use_zoomed_out_camera = !state.use_zoomed_out_camera;
    }
  }

  // Toggling music
  {
    if (did_press_key(tKey::M)) {
      state.music_enabled = !state.music_enabled;
    }
  }

  // Toggling slow motion
  {
    if (did_press_key(tKey::ARROW_LEFT)) {
      state.use_slow_motion = true;

      show_overlay_message("Slow motion ON");
    }

    if (did_press_key(tKey::ARROW_RIGHT)) {
      state.use_slow_motion = false;

      show_overlay_message("Slow motion OFF");
    }
  }

  // Resetting entities
  {
    if (did_press_key(tKey::R)) {
      for_entities(state.light_posts) {
        auto& entity = state.light_posts[i];

        entity.did_activate = false;
        entity.astro_activation_time = 0.f;
        entity.game_activation_time = -1.f;
        entity.is_astro_synced = false;
      }

      for_entities(state.gates) {
        auto& entity = state.gates[i];

        entity.did_activate = false;
        entity.astro_activation_time = 0.f;
        entity.game_activation_time = -1.f;
      }

      for_entities(state.bird_gates) {
        auto& entity = state.bird_gates[i];

        entity.did_activate = false;
        entity.astro_activation_time = 0.f;
        entity.game_activation_time = -1.f;
      }

      for_entities(state.water_wheels) {
        auto& entity = state.water_wheels[i];

        entity.did_activate = false;
        entity.astro_activation_time = 0.f;
        entity.game_activation_time = -1.f;
      }

      for (auto type : { LESSER_GUARD, LOW_GUARD, FAERIE, NPC }) {
        for_entities_of_type(type) {
          auto& entity = entities[i];

          HardResetEntity(entity);
        }
      }

      for_entities(state.small_birds) {
        auto& entity = state.small_birds[i];

        SmallBird::Reset(entity);
      }

      for_entities(state.sculpture_1s) {
        auto& entity = state.sculpture_1s[i];

        if (entity.did_activate) {
          entity.did_activate = false;
          entity.game_activation_time = -1.;
          entity.astro_activation_time = 0.f;
        }

        if (entity.light_id != -1) {
          remove_point_light(entity.light_id);

          entity.light_id = -1;
        }
      }

      for (auto type : { EVENT_TRIGGER, ITEM_PICKUP }) {
        for_entities_of_type(type) {
          auto& entity = entities[i];

          entity.did_activate = false;
        }
      }

      show_overlay_message("Reset interactible entities");
    }
  }

  // Acquiring/unacquiring all items
  {
    if (did_press_key(tKey::I)) {
      if (Items::HasItem(state, MAGIC_WAND)) {
        show_overlay_message("Reset items");

        state.inventory.clear();
      } else {
        Items::CollectItem(tachyon, state, ITEM_STUN_SPELL);
        Items::CollectItem(tachyon, state, ITEM_HOMING_SPELL);
        Items::CollectItem(tachyon, state, GATE_KEY);
        Items::CollectItem(tachyon, state, MAGIC_WAND);
      }
    }
  }

  // Graphics toggling
  {
    if (did_press_key(tKey::G)) {
      auto& fx = tachyon->fx;

      if (fx.enable_shadows && fx.enable_ssao) {
        fx.enable_shadows = false;

        show_overlay_message("Shadows disabled");
      }
      else if (!fx.enable_shadows && fx.enable_ssao) {
        fx.enable_ssao = false;

        show_overlay_message("SSAO disabled");
      }
      else {
        fx.enable_shadows = true;
        fx.enable_ssao = true;

        show_overlay_message("Shadows and SSAO enabled");
      }
    }
  }
}

// @todo Environment::
static uint16 Hash(uint16 x) {
  x ^= x >> 8;
  x *= 0x352d;
  x ^= x >> 7;
  x *= 0xa68b;
  x ^= x >> 8;

  return x;
}

// @todo Environment::
static float HashToFloat(uint16 h) {
  return h / float(0xFFFF);
}

// @todo Environment::
static float Wrap(float value, float min, float max, float range) {
  if (value < 0.f) {
    return max + fmodf(value, range);
  } else {
    return min + fmodf(value, range);
  }
}

// @todo Environment::
static void HandleFog(Tachyon* tachyon, State& state) {
  auto& fx = tachyon->fx;

  // @temporary
  // @todo lerp based on day/night alpha
  fx.fog_visibility = state.is_nighttime ? 15000.f : 4000.f;
}

// @todo Environment::
static void HandleSnow(Tachyon* tachyon, State& state) {
  profile("HandleSnow()");

  float min_x = state.player_position.x - 12000.f;
  float max_x = state.player_position.x + 12000.f;
  float range_x = max_x - min_x;

  float min_z = state.player_position.z - 12000.f;
  float max_z = state.player_position.z + 12000.f;
  float range_z = max_z - min_z;

  float scale_alpha = Tachyon_InverseLerp(astro_time_periods.past, astro_time_periods.distant_past, state.astro_time);

  // Skip handling snow particles if they're too small to see
  if (scale_alpha == 0.f) return;

  tVec3f scale = tVec3f(25.f * scale_alpha);

  for (auto& particle : objects(state.meshes.snow_particle)) {
    float y = HashToFloat(Hash(particle.object_id)) * 10000.f - 10000.f;
    y -= get_scene_time() * 1000.f;
    y = fmodf(y, 10000.f) + 10000.f;

    float x = HashToFloat(Hash((particle.object_id + 1) * 4)) * 24000.f;
    float z = HashToFloat(Hash((particle.object_id + 1) * 12)) * 24000.f;

    float local_x = state.player_position.x - x;
    float local_z = state.player_position.z - z;

    tVec3f offset;
    offset.x = Wrap(-local_x, min_x, max_x, range_x);
    offset.y = y;
    offset.z = Wrap(-local_z, min_z, max_z, range_z);

    particle.position = offset;
    particle.scale = scale;
    particle.rotation = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), y * 0.001f);
    particle.color = tVec4f(0.4f, 0.6f, 1.f, 0.7f);
    particle.material = tVec4f(0.f, 1.f, 0, 1.f);

    commit(particle);
  }
}

// @todo 3d positioned sfx
static void HandleWalkSounds(Tachyon* tachyon, State& state) {
  // @todo use velocity
  bool is_running = is_key_held(tKey::CONTROLLER_A);
  float distance_threshold = is_running ? 2400.f : 1400.f;
  float last_sound_distance = state.movement_distance - state.last_walk_sound_movement_distance;

  if (last_sound_distance > distance_threshold) {
    auto cycle = state.walk_cycle++;
    float volume = is_running ? 0.1f : 0.05f;

    if (cycle == 0) {
      Sfx::PlaySound(SFX_GROUND_WALK_1, volume);
    }
    else if (cycle == 1) {
      Sfx::PlaySound(SFX_GROUND_WALK_2, volume);
    }
    else if (cycle == 2) {
      Sfx::PlaySound(SFX_GROUND_WALK_3, volume);

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

// @todo WindChimes::
static bool IsPlayerNearWindChimes(State& state) {
  for_entities(state.wind_chimes) {
    auto& entity = state.wind_chimes[i];
    float player_distance = tVec3f::distance(state.player_position, entity.position);

    if (player_distance < 7500.f) {
      return true;
    }
  }

  return false;
}

static void HandleMusicLevels(Tachyon* tachyon, State& state) {
  bool is_in_stealth_mode = IsInStealthMode(state);

  // Ambient sounds
  {
    Sound current_ambient_sound = GetCurrentAmbientSound(state);

    Sfx::LoopSound(current_ambient_sound, 0.5f);

    if (state.astro_turn_speed != 0.f || !state.music_enabled) {
      Sfx::FadeSoundVolumeTo(current_ambient_sound, 0.f, 500);
    } else {
      float volume = is_in_stealth_mode ? 0.8f : 0.6f;

      Sfx::FadeSoundVolumeTo(current_ambient_sound, volume, 500);
    }
  }

  // Background music
  {
    if (state.astro_turn_speed != 0.f || !state.music_enabled) {
      BGM::FadeCurrentMusicVolumeTo(0.f, 500);
    }
    else if (is_in_stealth_mode || Targeting::IsInCombatMode(state)) {
      BGM::FadeCurrentMusicVolumeTo(0.2f, 1000);
    }
    else if (state.wand_hold_factor > 0.5f) {
      BGM::FadeCurrentMusicVolumeTo(0.f, 1000);
    }
    else {
      BGM::FadeCurrentMusicVolumeTo(0.4f, 3000);
    }
  }
}

static void HandleCurrentAreaMusic(Tachyon* tachyon, State& state) {
  if (
    state.bgm_start_time == -1.f &&
    state.astro_time < 0.f && state.astro_turn_speed == 0.f
  ) {
    // Start music after astro traveling back the first time
    state.bgm_start_time = get_scene_time();
  }

  if (!state.music_enabled) {
    return;
  }

  // @temporary!!!!!
  // @todo BGM entities
  tVec3f village_position = tVec3f(232000.f, 0, 106000.f);

  if (IsPlayerNearWindChimes(state)) {
    BGM::LoopMusic(BGM_WIND_CHIMES, 0.4f);
  }
  else if (state.astro_time >= astro_time_periods.present) {
    // @temporary
    // @todo add overworld music for this period
    BGM::StopCurrentMusic();
  }
  // @temporary
  else if (tVec3f::distance(state.player_position, village_position) < 40000.f) {
    BGM::LoopMusic(VILLAGE_1, 0.4f);
  }
  else if (state.bgm_start_time != -1.f) {
    BGM::LoopMusic(DIVINATION_WOODREALM, 0.4f);
  }
}

static void ShowGameStats(Tachyon* tachyon, State& state) {
  float player_speed = state.player_velocity.magnitude();

  std::string stat_messages[] = {
    "Player " + state.player_position.toString(),
    "HP " + std::to_string(state.player_hp),
    "Speed " + std::to_string(player_speed),
    "Camera " + tachyon->scene.camera.position.toString(),
    "Astro time: " + std::to_string(state.astro_time),
    "Target time: " + std::to_string(state.target_astro_time),
    "Astro turn speed: " + std::to_string(state.astro_turn_speed),
    "Ground Y: " + std::to_string(state.current_ground_y),
    "Animation time: " + std::to_string(state.player_mesh_animation.seek_time),
    "Delta time: " + std::to_string(state.dt)
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

// @todo PlayerCharacter::
static void RecordPreviousPlayerPosition(State& state) {
  auto& positions = state.previous_player_positions;

  if (positions.size() == 10) {
    positions.erase(positions.begin());
  }

  positions.push_back(state.player_position);
}

// @todo PlayerCharacter::
static float GetNetMovementDistance(State& state) {
  tVec3f movement = tVec3f(0.f);

  for (size_t i = state.previous_player_positions.size() - 1; i > 0; i--) {
    auto& current = state.previous_player_positions[i];
    auto& previous = state.previous_player_positions[i - 1];

    tVec3f delta = previous - current;

    movement += delta;
  }

  return movement.magnitude();
}

static tVec3f GetLastUsedWindChimesPosition(State& state) {
  if (state.last_used_wind_chimes_id != -1) {
    EntityRecord record;
    record.type = WIND_CHIMES;
    record.id = state.last_used_wind_chimes_id;

    auto& entity = *EntityManager::FindEntity(state, record);

    return entity.position;
  }

  for (auto& entity : state.wind_chimes) {
    if (entity.unique_name == "game_start_chimes") {
      return entity.position;
    }
  }

  return tVec3f(0.f);
}

// @todo PlayerCharacter::RespawnPlayer()
static void RespawnPlayer(Tachyon* tachyon, State& state) {
  // @todo load from save
  tVec3f spawn_position = GetLastUsedWindChimesPosition(state);
  spawn_position.y = 1500.f + CollisionSystem::QueryGroundHeight(state, spawn_position.x, spawn_position.z);
  spawn_position.z += 4000.f;

  state.player_position = spawn_position;
  state.player_facing_direction = tVec3f(0, 0, 1.f);
  state.player_velocity = tVec3f(0.f);
  state.player_hp = 100.f;
  state.last_spawn_time = get_scene_time();

  state.previous_player_positions.clear();

  RecordPreviousPlayerPosition(state);

  // Reset camera
  state.camera_shift = tVec3f(0, 0, 1800.f);
  tachyon->scene.camera.position = spawn_position + tVec3f(0.f, 11000.f, 10800.f);

  // @temporary
  state.dismissed_blocking_dialogue = true;
  state.has_blocking_dialogue = false;
  state.dialogue_start_time = 0.f;

  // @todo factor
  for_entities(state.lesser_guards) {
    auto& entity = state.lesser_guards[i];

    HardResetEntity(entity);
  }

  // @todo factor
  for_entities(state.low_guards) {
    auto& entity = state.low_guards[i];

    HardResetEntity(entity);
  }

  Environment::Init(tachyon, state);
}

static void HandleFrameEnd(Tachyon* tachyon, State& state) {
  auto& scene = tachyon->scene;
  auto& fx = tachyon->fx;

  // Tracking net movement, updating the last player position
  {
    if (GetNetMovementDistance(state) > 50.f) {
      tVec3f last_move = state.player_position - state.previous_player_positions.back();
      float previous_move_delta = last_move.magnitude();

      state.previous_move_delta = previous_move_delta;
    } else {
      state.previous_move_delta = 0.f;
    }

    state.movement_distance += state.previous_move_delta;

    RecordPreviousPlayerPosition(state);
  }

  // Accumulation blur
  {
    float max_blur_factor = 0.98f - 5.f * state.dt;

    fx.accumulation_blur_factor = sqrtf(abs(state.astro_turn_speed)) * 4.1f;

    if (fx.accumulation_blur_factor > max_blur_factor) {
      fx.accumulation_blur_factor = max_blur_factor;
    }
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

  // Haze intensity
  // @todo move to time_evolution.cpp
  {
    float haze_alpha = Tachyon_InverseLerp(astro_time_periods.past, astro_time_periods.present, state.astro_time);

    fx.haze_intensity = Tachyon_Lerpf(0.2f, 0.3f, haze_alpha);
  }

  // Vignette effects in stealth mode
  {
    float desired_vignette_intensity = IsInStealthMode(state) ? 1.f : 0.25f;

    fx.vignette_intensity = Tachyon_Lerpf(fx.vignette_intensity, desired_vignette_intensity, 2.f * state.dt);
  }

  // Foliage collision
  {
    auto& velocity = state.player_velocity;
    float speed = velocity.magnitude();
    tVec3f foliage_movement_offset = (speed > 0.f ? velocity.invert().unit() * 500.f : 0.f);

    scene.foliage_mover_position = state.player_position + foliage_movement_offset;
    scene.foliage_mover_velocity = velocity;

    if (speed > 300.f) {
      scene.foliage_mover_velocity = velocity.unit() * 300.f;
    }

    // @hack
    scene.foliage_mover_velocity *= 1.5f;
  }

  // Disocclusion target
  {
    scene.disocclusion_target_position = state.player_position;
    scene.disocclusion_target_position.y += 1500.f;
  }

  // Astro turn direction tracking
  {
    if (state.astro_turn_speed != 0.f) {
      state.last_astro_turn_direction = state.astro_turn_speed > 0.f ? 1.f : -1.f;
    }
  }

  // @todo debug mode only
  {
    tachyon->show_timing_profile = state.show_game_stats;

    if (state.show_game_stats) {
      ShowGameStats(tachyon, state);
    }
  }
}

void astro::InitGame(Tachyon* tachyon, State& state) {
  MeshLibrary::AddMeshes(tachyon, state);

  // @todo factor
  {
    state.animations.player_idle.frames = {
      GltfLoader("./astro/3d_skeleton_animations/player_idle/idle_1.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_idle/idle_2.gltf").skeleton
    };

    state.animations.player_idle_wand.frames = {
      GltfLoader("./astro/3d_skeleton_animations/player_idle_wand/idle_1.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_idle_wand/idle_2.gltf").skeleton
    };

    state.animations.player_walk.frames = {
      GltfLoader("./astro/3d_skeleton_animations/player_walk/walk_1.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_walk/walk_2.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_walk/walk_3.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_walk/walk_4.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_walk/walk_5.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_walk/walk_6.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_walk/walk_7.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_walk/walk_8.gltf").skeleton
    };

    state.animations.player_walk_wand.frames = {
      GltfLoader("./astro/3d_skeleton_animations/player_walk_wand/walk_1.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_walk_wand/walk_2.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_walk_wand/walk_3.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_walk_wand/walk_4.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_walk_wand/walk_5.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_walk_wand/walk_6.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_walk_wand/walk_7.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_walk_wand/walk_8.gltf").skeleton
    };

    state.animations.player_run.frames = {
      GltfLoader("./astro/3d_skeleton_animations/player_run/run_1.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_run/run_2.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_run/run_3.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_run/run_4.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_run/run_5.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_run/run_6.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_run/run_7.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_run/run_8.gltf").skeleton
    };

    state.animations.player_run_wand.frames = {
      GltfLoader("./astro/3d_skeleton_animations/player_run_wand/run_1.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_run_wand/run_2.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_run_wand/run_3.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_run_wand/run_4.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_run_wand/run_5.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_run_wand/run_6.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_run_wand/run_7.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_run_wand/run_8.gltf").skeleton
    };

    state.animations.player_swing_wand.frames = {
      GltfLoader("./astro/3d_skeleton_animations/player_swing_wand/swing_1.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_swing_wand/swing_2.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_swing_wand/swing_3.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_swing_wand/swing_4.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_swing_wand/swing_5.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_swing_wand/swing_6.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_swing_wand/swing_7.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_swing_wand/swing_8.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_swing_wand/swing_9.gltf").skeleton
    };

    // @todo factor
    for (auto& bone : state.animations.player_idle.frames[0].bones) {
      state.player_mesh_animation.active_pose.bones.push_back(bone);
    }
  }

  // @todo factor
  {
    state.animations.person_idle.frames = {
      GltfLoader("./astro/3d_skeleton_animations/person_idle/idle_1.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/person_idle/idle_2.gltf").skeleton
    };

    state.animations.person_talking.frames = {
      GltfLoader("./astro/3d_skeleton_animations/person_talking/talk_1.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/person_talking/talk_2.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/person_talking/talk_3.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/person_talking/talk_4.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/person_talking/talk_5.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/person_talking/talk_6.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/person_talking/talk_7.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/person_talking/talk_8.gltf").skeleton,
    };

    state.animations.person_hit_front.frames = {
      GltfLoader("./astro/3d_skeleton_animations/person_hit_front/hit_1.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/person_hit_front/hit_2.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/person_hit_front/hit_3.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/person_hit_front/hit_4.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/person_hit_front/hit_5.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/person_hit_front/hit_6.gltf").skeleton
    };

    for_range(0, MAX_ANIMATED_PEOPLE - 1) {
      auto& skin = state.person_skinned_meshes[i];

      // @todo factor
      for (auto& bone : state.animations.person_idle.frames[0].bones) {
        skin.animation.active_pose.bones.push_back(bone);
      }
    }
  }

  // @todo ui.cpp
  state.debug_text = Tachyon_CreateUIText("./fonts/CascadiaMonoNF.ttf", 19);
  state.debug_text_large = Tachyon_CreateUIText("./fonts/OpenSans-Regular.ttf", 32);

  state.ui.future_age_title = Tachyon_CreateUIElement("./astro/textures/titles/future-age.png");
  state.ui.present_age_title = Tachyon_CreateUIElement("./astro/textures/titles/present-age.png");
  state.ui.past_age_title = Tachyon_CreateUIElement("./astro/textures/titles/past-age.png");
  state.ui.divination_woodrealm_title = Tachyon_CreateUIElement("./astro/textures/titles/divination-woodrealm.png");

  CreateConstantObjects(tachyon, state);

  DataLoader::LoadLevelData(tachyon, state);
  DataLoader::LoadNpcDialogue(tachyon, state);
  CollisionSystem::RebuildFlatGroundPlanes(tachyon, state);
  Items::SpawnItemObjects(tachyon, state);
  ProceduralBehavior::Generation::RebuildAllProceduralObjects(tachyon, state);
  EntityManager::CreateEntityAssociations(state);
  Particles::InitParticles(tachyon, state);
  RespawnPlayer(tachyon, state);

  state.player_light_id = create_point_light();
  state.astrolabe_light_id = create_point_light();

  tachyon->scene.scene_time = 0.f;
  tachyon->scene.use_close_camera_disocclusion = true;
}

void astro::UpdateGame(Tachyon* tachyon, State& state, const float dt) {
  profile("UpdateGame()");

  // @todo HandleFrameStart()
  {
    state.spells.did_cast_stun_this_frame = false;
    state.dt = dt;

    // @todo dev mode only
    if (state.use_slow_motion) {
      state.dt *= 0.2f;
    }

    tachyon->scene.scene_time += dt;
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

  // @todo dev mode only
  HandleInGameDevHotkeys(tachyon, state);

  TimeEvolution::HandleAstroTravel(state);
  GameEvents::HandleEvents(tachyon, state);
  Targeting::HandleTargets(tachyon, state);
  ControlSystem::HandleControls(tachyon, state);
  CollisionSystem::HandleCollisions(tachyon, state);
  SpellSystem::HandleSpells(tachyon, state);
  UISystem::HandleDialogue(tachyon, state);
  Particles::HandleParticles(tachyon, state);
  DynamicFauna::HandleBehavior(tachyon, state);
  FacadeGeometry::HandleFacades(tachyon, state);
  Environment::HandleEnvironment(tachyon, state);
  HandleFog(tachyon, state);
  HandleSnow(tachyon, state);
  HandleWalkSounds(tachyon, state);
  HandleCurrentAreaMusic(tachyon, state);
  HandleMusicLevels(tachyon, state);

  TimeEvolution::UpdateAstroTime(tachyon, state);
  ProceduralBehavior::Generation::UpdateProceduralObjects(tachyon, state);
  ProceduralBehavior::Growth::Update(tachyon, state);
  CameraSystem::UpdateCamera(tachyon, state);
  Astrolabe::Update(tachyon, state);
  UISystem::UpdateHUD(tachyon, state);
  PlayerCharacter::UpdatePlayer(tachyon, state);
  AnimatedEntities::UpdateAnimatedEntities(tachyon, state);
  UpdateWaterPlane(tachyon, state);
  UpdateLevelsOfDetail(tachyon, state);

  // @todo move to PlayerCharacter::UpdatePlayer()
  if (
    state.player_hp <= 0.f &&
    time_since(state.last_death_time) > 5.f
  ) {
    RespawnPlayer(tachyon, state);
  }

  HandleFrameEnd(tachyon, state);
}