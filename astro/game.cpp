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
#include "astro/hud_system.h"
#include "astro/items.h"
#include "astro/level_editor.h"
#include "astro/mesh_library.h"
#include "astro/particles.h"
#include "astro/player_animation.h"
#include "astro/player_character.h"
#include "astro/procedural_generation.h"
#include "astro/procedural_growth.h"
#include "astro/sfx.h"
#include "astro/sound_driver.h"
#include "astro/spell_system.h"
#include "astro/targeting.h"
#include "astro/time_evolution.h"
#include "astro/ui_system.h"
#include "astro/wand_abilities.h"

// @todo dev_tools.cpp
#include "astro/entity_behaviors/SmallBird.h"

using namespace astro;

static void CreateConstantObjects(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  create(meshes.player_head);
  create(meshes.player_wand);
  create(meshes.player_satchel);
  create(meshes.player_blanket);
  create(meshes.player_flask);
  create(meshes.player_flask);
  create(meshes.player_lantern);

  create(meshes.water_plane);

  // Environment meshes
  {
    for_range(1, 100) {
      // create(meshes.snow_particle);
    }

    for_range(1, 30) {
      create(meshes.stray_leaf);
    }

    for_range(1, 1000) {
      create(meshes.dust_mote);
    }

    for_range(1, 500) {
      create(meshes.river_leaf);
    }

    for_range(1, 10) {
      create(meshes.dust_cloud);
    }
  }

  // Dynamic fauna meshes
  {
    for_range(1, 50) {
      // Tiny birds
      create(meshes.tiny_bird_head);
      create(meshes.tiny_bird_body);
      create(meshes.tiny_bird_wings);
      create(meshes.tiny_bird_left_wing);
      create(meshes.tiny_bird_right_wing);

      // Ducks
      create(meshes.duck_body);
      create(meshes.duck_neck);
      create(meshes.duck_wings);
      create(meshes.duck_head);
      create(meshes.duck_beak);

      // Swans
      create(meshes.swan_body);
      create(meshes.swan_beak);
      create(meshes.swan_beak_skin);
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

  for_range(1, 100) {
    create(meshes.ladder_rung);
  }

  for_range(1, 200) {
    create(meshes.stair_step);
  }

  for_range(1, 100) {
    create(meshes.castle_tile);
  }

  // Clothing + armor
  {
    for_range(1, 10) {
      create(meshes.lesser_helmet);
      create(meshes.low_helmet);
    }

    for_range(1, 20) {
      create(meshes.lesser_vambrace);
      create(meshes.shoulder_plate);
    }
  }

  // HUD objects
  {
    create(meshes.astrolabe_rear);
    create(meshes.astrolabe_base);
    create(meshes.astrolabe_plate);
    create(meshes.astrolabe_plate2);
    create(meshes.astrolabe_plate3);
    create(meshes.astrolabe_plate4);
    create(meshes.astrolabe_ring);
    create(meshes.astrolabe_hand);
    create(meshes.health_bar);

    for_range(1, 4) {
      create(meshes.health_unit);
    }

    create(meshes.target_reticle);
  }

  create(meshes.item_gate_key);


  // @todo dev mode only
  {
    for (uint16 i = 0; i < 100; i++) {
      create(meshes.debug_skeleton_bone);
    }

    for_range(1, 100) {
      create(meshes.debug_collision_point);
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
    Tachyon_UseLodByDistance(tachyon, meshes.rock_1, 50000.f, 100000.f);
    Tachyon_UseLodByDistance(tachyon, meshes.rock_2, 50000.f);
    Tachyon_UseLodByDistance(tachyon, meshes.river_edge, 50000.f, 100000.f);
    Tachyon_UseLodByDistance(tachyon, meshes.ground_1, 50000.f, 100000.f);
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
static void ResetEntities(Tachyon* tachyon, State& state) {
  state.player.is_hopping_up_to_climb_down = false;

  // Reset wand pulses to avoid re-activating anything
  {
    tachyon->fx.wand_pulse_alpha = 1.f;
    state.last_wand_light_pulse_time = 0.f;
  }

  for_entities(state.light_posts) {
    auto& entity = state.light_posts[i];

    entity.did_activate = false;
    entity.astro_activation_time = 0.f;
    entity.game_activation_time = -1.f;
    entity.is_astro_synced = false;
  }

  for (auto type : { GATE, BIRD_GATE, IRON_GATE, WATER_WHEEL, CASTLE_STAIRS, NORMAL_SWITCH }) {
    for_entities_of_type(type) {
      auto& entity = entities[i];

      entity.did_activate = false;
      entity.astro_activation_time = 0.f;
      entity.game_activation_time = -1.f;
      entity.can_activate = true;
    }
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

  state.tiny_birds.clear();

  for_entities(state.sculpture_1s) {
    auto& entity = state.sculpture_1s[i];

    // Don't reset sculptures which are perma-activated
    if (entity.requires_action) continue;

    if (entity.did_activate) {
      entity.did_activate = false;
      entity.game_activation_time = -1.f;
      entity.astro_activation_time = 0.f;
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

    if (did_press_key(tKey::CONTROLLER_B)) {
      state.use_slow_motion = !state.use_slow_motion;

      if (state.use_slow_motion) {
        show_overlay_message("Slow motion ON");
      } else {
        show_overlay_message("Slow motion OFF");
      }
    }
  }

  // Resetting entities
  {
    if (did_press_key(tKey::R)) {
      ResetEntities(tachyon, state);
    }
  }

  // Acquiring/unacquiring all items
  // @todo make this work properly
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

  // Enemy toggling
  {
    if (did_press_key(tKey::BACKSPACE)) {
      state.enemies_disabled = !state.enemies_disabled;

      if (state.enemies_disabled) {
        show_overlay_message("Disabled enemies");
      } else {
        show_overlay_message("Re-enabled enemies");
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
static void UseMediumHazeColor(Tachyon::Fx& fx, const tVec3f& color, const float blend_alpha) {
  fx.medium_haze_color = tVec3f::lerp(fx.medium_haze_color, color, blend_alpha);
}

// @todo Environment::
static void HandleFog(Tachyon* tachyon, State& state) {
  auto& fx = tachyon->fx;

  // @temporary
  // @todo lerp based on day/night alpha
  fx.fog_volume_visibility = state.is_nighttime ? 15000.f : 4000.f;

  // Medium haze
  // @todo HandleMediumHaze()
  {
    // Present + Future
    if (state.astro_time >= astro_time_periods.present) {
      if (state.current_location == Location::TUTORIAL) {
        UseMediumHazeColor(fx, tVec3f(1.f, 0.6f, 1.f), state.dt);
      } else {
        UseMediumHazeColor(fx, tVec3f(1.6f, 0.8f, 0.6f), state.dt);
      }
    }

    // Past
    else if (state.astro_time == astro_time_periods.past) {
      if (state.current_location == Location::DIVINATION_LAKE_PROMENADE) {
        UseMediumHazeColor(fx, tVec3f(2.f, 0.9f, 0.6f), state.dt);
      } else {
        UseMediumHazeColor(fx, tVec3f(2.f, 1.2f, 0.6f), state.dt);
      }
    }

    // Distant past
    else if (state.astro_time == astro_time_periods.distant_past) {
      UseMediumHazeColor(fx, tVec3f(1.6f, 0.8f, 1.f), state.dt);
    }
  }
}

// @todo Environment::
// @todo remove? replace with rain? (preserve the wrapping behavior though)
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

// @todo move to engine
template<class T>
static inline void RemoveFromArray(std::vector<T>& array, uint32 index) {
  array.erase(array.begin() + index);
}

static void HandleDustClouds(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  reset_instances(meshes.dust_cloud);

  for (int i = state.dust_clouds.size() - 1; i >= 0; i--) {
    auto& cloud = state.dust_clouds[i];
    float time_since_spawned = time_since(cloud.spawn_time);

    float alpha = time_since_spawned / 0.75f;
    clamp_to_1(alpha);

    if (alpha == 1.f) {
      // Ignore and remove expired dust clouds
      RemoveFromArray(state.dust_clouds, i);

      continue;
    }

    auto& object = use_instance(meshes.dust_cloud);

    object.position = cloud.spawn_position;
    object.position.y += sqrtf(alpha) * 500.f;

    object.scale = tVec3f(500.f * (1.f - alpha));

    object.color = tVec4f(1.f, 1.f, 1.f, 0.7f);
    object.material = tVec4f(1.f, 0, 0, 1.f);

    commit(object);
  }
}

// @todo 3d positioned sfx
static void HandleWalkSounds(Tachyon* tachyon, State& state) {
  // Don't play sounds if we're on a ladder or in freefall
  if (
    state.is_on_ladder ||
    state.did_jump_off_ledge ||
    state.player_position.y > state.current_ground_y
  ) {
    return;
  }

  float player_speed = get_speed();

  if (player_speed < 50.f || time_since(state.last_walk_sound_time) < 0.2f) {
    // Don't play walk sounds if we're not moving fast enough,
    // or if we just played one of the sounds
    return;
  }

  bool is_running = player_speed > PlayerCharacter::MAX_COMBAT_WALK_SPEED;
  // @todo Why are the running/walking step frames desynced here?
  // They appear to be the same in the actual animation.
  int step_frame_1 = is_running ? 2 : 0; // Left foot
  int step_frame_2 = is_running ? 6 : 4; // Right foot
  float seek_time = PlayerAnimation::GetRunCycleAnimationTime(state);
  int current_frame = (int) fmodf(seek_time, 8.f);

  if (
    seek_time > 0.f &&
    current_frame == step_frame_1 ||
    current_frame == step_frame_2
  ) {
    auto cycle = state.walk_cycle++;

    float volume = (
      is_running ? 0.2f :
      PlayerCharacter::IsClimbingOffLadder(tachyon, state) ? 0.25f :
      0.1f
    );

    if (state.is_on_stone_surface) {
      volume *= 2.f;
    }

    SoundDriver::PlayWalkSound(state, volume);

    state.last_walk_sound_time = get_scene_time();
  }
}

static void HandleClimbingSounds(Tachyon* tachyon, State& state) {
  // Don't play climbing sounds if:
  if (
    // We're not on a ladder
    !state.is_on_ladder ||
    // We're hopping up onto a ladder, but not on it yet
    state.player.is_hopping_up_to_climb_down ||
    // We just played a sound
    time_since(state.last_walk_sound_time) < 0.2f ||
    // We're not moving up or down the ladder
    (!is_moving_left_stick() && !state.player.is_starting_climb_down)
  ) {
    return;
  }

  int step_frame_1 = 0; // Left foot
  int step_frame_2 = 4; // Right foot
  float seek_time = state.player.rig.next_animation_time;
  int current_frame = (int) fmodf(seek_time, 8.f);

  if (current_frame == step_frame_1 || current_frame == step_frame_2) {
    auto cycle = state.walk_cycle++;
    float volume = state.player.is_starting_climb_down ? 1.25f : 0.75f;

    SoundDriver::PlayLadderSound(state, volume);

    state.last_walk_sound_time = get_scene_time();
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

static bool IsPlayerNearUsableWindChimes(State& state) {
  for_entities(state.wind_chimes) {
    auto& entity = state.wind_chimes[i];

    // Ignore wind chimes which aren't repaired yet
    if (entity.requires_action && !entity.is_astro_synced) continue;

    float player_distance = tVec3f::distance(state.player_position, entity.position);

    if (player_distance < 7500.f) {
      return true;
    }
  }

  return false;
}

const static float BGM_VOLUME = 0.5f;

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
    else if (state.is_holding_up_wand) {
      BGM::FadeCurrentMusicVolumeTo(0.1f, 1000);
    }
    else {
      BGM::FadeCurrentMusicVolumeTo(BGM_VOLUME, 3000);
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

  if (IsPlayerNearUsableWindChimes(state)) {
    BGM::LoopMusic(BGM_WIND_CHIMES, BGM_VOLUME);
  }
  else if (
    state.astro_time >= astro_time_periods.present ||
    state.astro_time == astro_time_periods.distant_past
  ) {
    // @temporary
    // @todo add overworld music for this period
    BGM::StopCurrentMusic();
  }
  // @temporary
  else if (tVec3f::distance(state.player_position, village_position) < 40000.f) {
    BGM::LoopMusic(BGM_VILLAGE_1, BGM_VOLUME);
  }
  else if (state.bgm_start_time != -1.f) {
    if (state.current_location == Location::DIVINATION_LAKE_PROMENADE) {
      BGM::LoopMusic(BGM_PROMENADE, BGM_VOLUME);
    } else {
      BGM::LoopMusic(BGM_DIVINATION_WOODREALM, BGM_VOLUME);
    }
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
    "Current animation: " + state.player.rig.current_animation->name,
    "Next animation: " + state.player.rig.next_animation->name,
    "Current animation time: " + std::to_string(state.player.rig.current_animation_time),
    "Next animation time: " + std::to_string(state.player.rig.next_animation_time),
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
  state.current_location = state.last_wind_chimes_location;

  // Reset previous player positions tracking
  state.previous_player_positions.clear();

  RecordPreviousPlayerPosition(state);

  // Reset camera
  state.camera_tracking_position = state.player_position;
  state.camera_offset_position = state.player_facing_direction * 1500.f;

  // @todo factor
  tachyon->scene.camera.position =
    state.camera_tracking_position +
    state.camera_offset_position +
    tVec3f(0.f, 11000.f, 8500.f);

  // @todo factor
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

    scene.foliage_mover_position = state.player_position;
    scene.foliage_mover_velocity = velocity;
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

    state.animations.player_idle.name = "PLAYER_IDLE";

    state.animations.player_idle_quickturn.frames = {
      GltfLoader("./astro/3d_skeleton_animations/player_idle_quickturn/idle_quickturn_1.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_idle_quickturn/idle_quickturn_2.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_idle_quickturn/idle_quickturn_3.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_idle_quickturn/idle_quickturn_4.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_idle_quickturn/idle_quickturn_5.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_idle_quickturn/idle_quickturn_6.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_idle_quickturn/idle_quickturn_7.gltf").skeleton,
    };

    state.animations.player_idle_quickturn.name = "PLAYER_IDLE_QUICKTURN";

    state.animations.player_idle_2.frames = {
      GltfLoader("./astro/3d_skeleton_animations/player_idle_2/idle_1.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_idle_2/idle_2.gltf").skeleton
    };

    state.animations.player_idle_2.name = "PLAYER_IDLE_2";

    state.animations.player_idle_wand.frames = {
      GltfLoader("./astro/3d_skeleton_animations/player_idle_wand/idle_1.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_idle_wand/idle_2.gltf").skeleton
    };

    state.animations.player_idle_wand.name = "PLAYER_IDLE_WAND";

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

    state.animations.player_walk.name = "PLAYER_WALK";

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

    state.animations.player_walk_wand.name = "PLAYER_WALK_WAND";

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

    state.animations.player_run.name = "PLAYER_RUN";

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

    state.animations.player_run_wand.name = "PLAYER_RUN_WAND";

    state.animations.player_climb.frames = {
      GltfLoader("./astro/3d_skeleton_animations/player_climb/climb_1.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_climb/climb_2.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_climb/climb_3.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_climb/climb_4.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_climb/climb_5.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_climb/climb_6.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_climb/climb_7.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_climb/climb_8.gltf").skeleton
    };

    state.animations.player_climb.name = "PLAYER_CLIMB";

    state.animations.player_climb_up.frames = {
      GltfLoader("./astro/3d_skeleton_animations/player_climb_up/climb_up_1.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_climb_up/climb_up_2.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_climb_up/climb_up_3.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_climb_up/climb_up_4.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_climb_up/climb_up_5.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_climb_up/climb_up_6.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_climb_up/climb_up_7.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_climb_up/climb_up_8.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_climb_up/climb_up_9.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_climb_up/climb_up_10.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_climb_up/climb_up_11.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_climb_up/climb_up_12.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_climb_up/climb_up_13.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_climb_up/climb_up_14.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_climb_up/climb_up_15.gltf").skeleton
    };

    state.animations.player_climb_up.looping = false;
    state.animations.player_climb_up.use_root_motion = true;
    state.animations.player_climb_up.name = "PLAYER_CLIMB_UP";

    state.animations.player_climb_up_jump.frames = {
      GltfLoader("./astro/3d_skeleton_animations/player_climb_up_jump/climb_up_jump_1.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_climb_up_jump/climb_up_jump_2.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_climb_up_jump/climb_up_jump_3.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_climb_up_jump/climb_up_jump_4.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_climb_up_jump/climb_up_jump_5.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_climb_up_jump/climb_up_jump_6.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_climb_up_jump/climb_up_jump_7.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_climb_up_jump/climb_up_jump_8.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_climb_up_jump/climb_up_jump_9.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_climb_up_jump/climb_up_jump_10.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_climb_up_jump/climb_up_jump_11.gltf").skeleton
    };

    state.animations.player_climb_up_jump.looping = false;
    state.animations.player_climb_up_jump.use_root_motion = true;
    state.animations.player_climb_up_jump.name = "PLAYER_CLIMB_UP_JUMP";

    state.animations.player_climb_down_onto.frames = {
      GltfLoader("./astro/3d_skeleton_animations/player_climb_down_onto/climb_down_onto_1.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_climb_down_onto/climb_down_onto_2.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_climb_down_onto/climb_down_onto_3.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_climb_down_onto/climb_down_onto_4.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_climb_down_onto/climb_down_onto_5.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_climb_down_onto/climb_down_onto_6.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_climb_down_onto/climb_down_onto_7.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_climb_down_onto/climb_down_onto_8.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_climb_down_onto/climb_down_onto_9.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_climb_down_onto/climb_down_onto_10.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_climb_down_onto/climb_down_onto_11.gltf").skeleton
    };

    state.animations.player_climb_down_onto.looping = false;
    state.animations.player_climb_down_onto.name = "PLAYER_CLIMB_DOWN_ONTO";

    state.animations.player_climb_down_off.frames = {
      GltfLoader("./astro/3d_skeleton_animations/player_climb_down/climb_down_1.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_climb_down/climb_down_2.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_climb_down/climb_down_3.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_climb_down/climb_down_4.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_climb_down/climb_down_5.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_climb_down/climb_down_6.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_climb_down/climb_down_7.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_climb_down/climb_down_8.gltf").skeleton
    };

    state.animations.player_climb_down_off.looping = false;
    state.animations.player_climb_down_off.name = "PLAYER_CLIMB_DOWN_OFF";

    state.animations.player_small_hop.frames = {
      GltfLoader("./astro/3d_skeleton_animations/player_small_hop/small_hop_1.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_small_hop/small_hop_2.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_small_hop/small_hop_3.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_small_hop/small_hop_4.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_small_hop/small_hop_5.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_small_hop/small_hop_6.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_small_hop/small_hop_7.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_small_hop/small_hop_8.gltf").skeleton
    };

    state.animations.player_small_hop.looping = false;
    state.animations.player_small_hop.name = "PLAYER_SMALL_HOP";

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

    state.animations.player_swing_wand.name = "PLAYER_SWING_WAND";

    state.animations.player_freefall.frames = {
      GltfLoader("./astro/3d_skeleton_animations/player_freefall/freefall_1.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_freefall/freefall_2.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_freefall/freefall_3.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_freefall/freefall_4.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_freefall/freefall_5.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_freefall/freefall_6.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_freefall/freefall_7.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_freefall/freefall_8.gltf").skeleton
    };

    state.animations.player_freefall.name = "PLAYER_FREEFALL";

    state.animations.player_freefall2.frames = {
      GltfLoader("./astro/3d_skeleton_animations/player_freefall2/freefall_1.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_freefall2/freefall_2.gltf").skeleton
    };

    state.animations.player_freefall2.name = "PLAYER_FREEFALL_2";

    state.animations.player_quick_slowdown.frames = {
      GltfLoader("./astro/3d_skeleton_animations/player_quick_slowdown/quick_slowdown_1.gltf").skeleton,
      GltfLoader("./astro/3d_skeleton_animations/player_quick_slowdown/quick_slowdown_2.gltf").skeleton
    };

    state.animations.player_quick_slowdown.name = "PLAYER_QUICK_SLOWDOWN";

    // @todo factor
    for (auto& bone : state.animations.player_idle.frames[0].bones) {
      state.player.rig.active_pose.bones.push_back(bone);
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
      auto& person = state.skinned_people[i];

      // @todo factor
      for (auto& bone : state.animations.person_idle.frames[0].bones) {
        person.rig.active_pose.bones.push_back(bone);
      }
    }
  }

  // @todo UISystem::
  {
    state.debug_text = Tachyon_CreateUIText("./fonts/CascadiaMonoNF.ttf", 19);
    state.debug_text_large = Tachyon_CreateUIText("./fonts/GoogleSans-Regular.ttf", 32);

    state.ui.future_age_title = Tachyon_CreateUIElement("./astro/textures/titles/future-age.png");
    state.ui.present_age_title = Tachyon_CreateUIElement("./astro/textures/titles/present-age.png");
    state.ui.past_age_title = Tachyon_CreateUIElement("./astro/textures/titles/past-age.png");
    state.ui.distant_past_age_title = Tachyon_CreateUIElement("./astro/textures/titles/distant-past-age.png");

    state.ui.divination_woodrealm_title = Tachyon_CreateUIElement("./astro/textures/titles/divination-woodrealm.png");
    state.ui.divination_riverway_title = Tachyon_CreateUIElement("./astro/textures/titles/divination-riverway.png");
    state.ui.lake_promenade_title = Tachyon_CreateUIElement("./astro/textures/titles/lake-promenade.png");
    state.ui.lakefront_south_title = Tachyon_CreateUIElement("./astro/textures/titles/lakefront-south.png");
    state.ui.garden_of_monastics_title = Tachyon_CreateUIElement("./astro/textures/titles/garden-of-monastics.png");

    state.ui.seeker_stargazer_title = Tachyon_CreateUIElement("./astro/textures/titles/seeker-stargazer.png");
  }

  CreateConstantObjects(tachyon, state);

  DataLoader::LoadLevelData(tachyon, state);
  DataLoader::LoadNpcDialogue(tachyon, state);
  DataLoader::LoadCameraData(tachyon, state);
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

    tachyon->scene.scene_time += state.dt;
  }

  // Toggle level editor with E
  // @todo dev mode only
  {
    if (did_press_key(tKey::E)) {
      if (state.is_level_editor_open) {
        LevelEditor::CloseLevelEditor(tachyon, state);
        Environment::Init(tachyon, state);
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

  TimeEvolution::HandleAstroTravel(tachyon, state);
  GameEvents::HandleEvents(tachyon, state);
  Targeting::HandleTargets(tachyon, state);
  ControlSystem::HandleControls(tachyon, state);
  CollisionSystem::HandleCollisions(tachyon, state);
  WandAbilities::HandleWandAbilities(tachyon, state);
  SpellSystem::HandleSpells(tachyon, state);
  UISystem::HandleDialogue(tachyon, state);
  Particles::HandleParticles(tachyon, state);
  DynamicFauna::HandleBehavior(tachyon, state);
  FacadeGeometry::HandleFacades(tachyon, state);
  Environment::HandleEnvironment(tachyon, state);
  HandleFog(tachyon, state);
  // HandleSnow(tachyon, state);
  HandleDustClouds(tachyon, state);
  HandleWalkSounds(tachyon, state);
  HandleClimbingSounds(tachyon, state);
  HandleCurrentAreaMusic(tachyon, state);
  HandleMusicLevels(tachyon, state);

  TimeEvolution::UpdateAstroTime(tachyon, state);
  ProceduralBehavior::Generation::UpdateProceduralObjects(tachyon, state);
  ProceduralBehavior::Growth::Update(tachyon, state);
  CameraSystem::UpdateCamera(tachyon, state);
  Astrolabe::Update(tachyon, state);
  UISystem::UpdateHUD(tachyon, state);
  HUDSystem::Update(tachyon, state);
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