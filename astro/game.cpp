#include "astro/game.h"
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
  water_plane.scale = tVec3f(200000.f, 1.f, 200000.f);
  water_plane.color = tVec3f(0, 0.1f, 0.3f);
  water_plane.material = tVec4f(0.1f, 1.f, 0, 0.5f);

  water_plane.position.y = -1800.f + 22.f * state.astro_time;

  if (water_plane.position.y > -1800.f) water_plane.position.y = -1800.f;
  if (water_plane.position.y < -4000.f) water_plane.position.y = -4000.f;

  commit(water_plane);

  state.water_level = water_plane.position.y;
}

// @todo cleanup
// @todo move elsewhere
//
// @todo figure out a better way to display the astrolabe
// orthographically + with the correct aspect ratio, still
// allowing it to receive lighting somehow?
static void UpdateAstrolabe(Tachyon* tachyon, State& state) {
  auto& camera = tachyon->scene.camera;
  auto& meshes = state.meshes;

  auto& rear = objects(meshes.astrolabe_rear)[0];
  auto& base = objects(meshes.astrolabe_base)[0];
  auto& plate = objects(meshes.astrolabe_plate)[0];
  auto& ring = objects(meshes.astrolabe_ring)[0];
  auto& hand = objects(meshes.astrolabe_hand)[0];

  rear.scale =
  base.scale =
  plate.scale =
  ring.scale =
  hand.scale =
  tVec3f(200.f);

  rear.color = tVec3f(0.1f);
  rear.material = tVec4f(0, 1.f, 0, 0);

  base.color = tVec3f(0.7f, 0.4f, 0.1f);
  base.material = tVec4f(0, 1.f, 1.f, 0.4f);

  plate.color = 0x4110;
  plate.material = tVec4f(0, 0, 0, 0);

  ring.color = tVec3f(0.9f, 0.8f, 0.1f);
  ring.material = tVec4f(0, 1.f, 0, 0.1f);

  hand.color = tVec3f(0.4f, 0.1f, 0.1f);
  hand.material = tVec4f(0.1f, 1.f, 0, 0);

  rear.rotation =
  base.rotation =
  plate.rotation =
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

  // Ring behavior
  {
    float ring_angle = -state.astro_time * 0.0412f - 0.04f;

    if (state.astro_time < -76.f) {
      // @hack Tweaky alignment stuff for the rotating ring
      //
      // @todo as unscrupulous as this correction is, we might be able
      // to get away with correcting the hand rotation as well and then
      // using equidistant min/max time segments, which would bring us
      // closer to normalizing some of this.
      float correction = state.astro_time + 76.f;

      ring_angle += correction * 0.0022f;
    }

    ring.rotation =
    (
      base.rotation *
      Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), ring_angle)
    );
  }

  rear.position =
  base.position =
  plate.position =
  ring.position =
  hand.position =
  (
    camera.position +
    camera.rotation.getDirection() * tVec3f(1.f, -1.f, 1.f) * 2000.f +
    camera.rotation.getLeftDirection() * 1200.f +
    camera.rotation.getUpDirection() * tVec3f(1.f, -1.f, 1.f) * 580.f
  );

  rear.position += (rear.position - camera.position).unit() * 20.f;
  rear.scale = tVec3f(205.f);
  rear.position += camera.rotation.getLeftDirection() * 1.f;
  rear.position += camera.rotation.getUpDirection() * 18.f;

  hand.position -= camera.rotation.getLeftDirection() * 6.f;

  // Fragments
  {
    auto& fragment_ul = objects(meshes.astrolabe_fragment_ul)[0];
    auto& fragment_ll = objects(meshes.astrolabe_fragment_ll)[0];

    fragment_ul.position = base.position;
    fragment_ul.scale = base.scale;
    fragment_ul.rotation = base.rotation;
    fragment_ul.color = tVec3f(0.7f, 0.5f, 0.2f);
    fragment_ul.material = base.material;

    if (Items::HasItem(state, ASTROLABE_LOWER_LEFT)) {
      fragment_ll.position = base.position;
      fragment_ll.scale = base.scale;
      fragment_ll.rotation = base.rotation;
      fragment_ll.color = tVec3f(0.7f, 0.5f, 0.2f);
      fragment_ll.material = base.material;
    }

    commit(fragment_ul);
    commit(fragment_ll);
  }

  // Add light for visibility
  {
    // @temporary
    // @todo put a light id in state
    // @todo destroy light when opening the editor
    static int32 light_id = -1;

    if (light_id == -1) {
      light_id = create_point_light();
    }

    auto* light = get_point_light(light_id);

    light->position = base.position + tVec3f(-10.f, -4.f, 8.f);
    light->radius = 300.f;
    light->color = tVec3f(1.f, 0.8f, 0.4f);
    light->power = 0.5f + 20.f * abs(state.astro_turn_speed);
    light->glow_power = 0.f;
  }

  commit(rear);
  commit(base);
  commit(plate);
  commit(ring);
  commit(hand);
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
  Tachyon_UseLodByDistance(tachyon, meshes.flower, 35000.f);

  // Entity parts
  Tachyon_UseLodByDistance(tachyon, meshes.oak_tree_roots, 40000.f);
  Tachyon_UseLodByDistance(tachyon, meshes.oak_tree_trunk, 40000.f);
  Tachyon_UseLodByDistance(tachyon, meshes.oak_tree_branches, 40000.f);

  Tachyon_UseLodByDistance(tachyon, meshes.flowers_stalks, 35000.f);
  Tachyon_UseLodByDistance(tachyon, meshes.flowers_petals, 35000.f);
}

static void ShowHighestLevelsOfDetail(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  Tachyon_ShowHighestLevelsOfDetail(tachyon, meshes.rock_1);
  Tachyon_ShowHighestLevelsOfDetail(tachyon, meshes.ground_1);
  Tachyon_ShowHighestLevelsOfDetail(tachyon, meshes.grass);
  Tachyon_ShowHighestLevelsOfDetail(tachyon, meshes.small_grass);
  Tachyon_ShowHighestLevelsOfDetail(tachyon, meshes.flower);
}

static void HandleWalkSounds(Tachyon* tachyon, State& state) {
  float player_speed = state.player_velocity.magnitude();

  if (player_speed < 200.f) {
    return;
  }

  // @todo base cycle time on player speed
  // @todo 3d positioned sfx
  if (tachyon->running_time - state.last_walk_sound_time > 0.3f) {
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

    state.last_walk_sound_time = tachyon->running_time;
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
}

void astro::UpdateGame(Tachyon* tachyon, State& state, const float dt) {
  profile("UpdateGame()");

  auto& scene = tachyon->scene;

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

  Targeting::HandleTargets(tachyon, state);
  ControlSystem::HandleControls(tachyon, state, dt);
  CollisionSystem::HandleCollisions(tachyon, state);
  SpellSystem::HandleSpells(tachyon, state, dt);
  Items::HandleItemPickup(tachyon, state);
  HandleDialogue(tachyon, state);
  HandleWalkSounds(tachyon, state);

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

    // tachyon->scene.scene_time += dt;

    // @todo ui.cpp
    // @todo debug mode only
    ShowGameStats(tachyon, state);
  }
}