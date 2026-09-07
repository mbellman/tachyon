#include "engine/tachyon.h"

#include "metro/game.h"
#include "metro/background_vehicles.h"
#include "metro/camera_system.h"
#include "metro/control_system.h"
#include "metro/interactive_entities.h"
#include "metro/player.h"
#include "metro/player_vehicle.h"
#include "metro/static_entities.h"
#include "metro/utilities.h"
#include "metro/world_editor.h"
#include "metro/world_init.h"

using namespace metro;

static void HandleFrameStart(Tachyon* tachyon, State& state, const float dt) {
  if (state.use_slow_motion) {
    state.dt = dt * 0.25f;
  } else {
    state.dt = dt;
  }

  tachyon->scene.scene_time += state.dt;
}

static void HandleDevHotkeys(Tachyon* tachyon, State& state) {
  // Toggle editor
  if (did_press_key(tKey::E)) {
    if (state.is_editor_open) {
      WorldEditor::Close(tachyon, state);
    } else {
      WorldEditor::Open(tachyon, state);
    }
  }

  // Toggle function timings
  if (did_press_key(tKey::SPACE)) {
    tachyon->show_timing_profile = !tachyon->show_timing_profile;
  }

  // Toggle slow motion
  if (did_press_key(GAMEPAD_O)) {
    state.use_slow_motion = !state.use_slow_motion;
  }

  // Toggle frame-by-frame
  if (did_press_key(tKey::CONTROLLER_L1)) {
    state.use_frame_stepping = !state.use_frame_stepping;
  }

  // Advance one frame at a time
  if (
    state.use_frame_stepping && (
      did_press_key(tKey::ARROW_RIGHT) ||
      did_press_key(tKey::CONTROLLER_R1)
    )
  ) {
    state.allow_frame_step = true;
  }

  // Respawn bike at start
  if (did_press_key(tKey::R)) {
    auto* active_vehicle = GetActiveVehicle(state);

    if (GetVehicleCategory(active_vehicle) == BICYCLE) {
      auto& bike = as_bicycle(active_vehicle);

      // @temporary
      // @todo create a method for resetting motion/rotation etc.
      bike.position = tVec3f(0, -2220.f, -10000.f);
      bike.visual_position = bike.position;
      bike.pedal_speed = 0.f;
      bike.speed = 0.f;
      bike.pitch = 0.f;
      bike.facing_direction = tVec3f(0, 0, -1.f);
      bike.drifting_factor = 0.f;
      bike.steering_angle = 0.f;
      bike.leaning_angle = 0.f;

      bike.flat_rotation =
        Quaternion::FromDirection(bike.facing_direction, Y_UP) *
        Quaternion::fromAxisAngle(AXIS_Z, bike.leaning_angle);

      bike.directional_rotation = bike.flat_rotation;
      bike.visual_rotation = bike.flat_rotation;
    }
  }
}

static void EnableEditorOnlyMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  objects(meshes.walkway_segment).disabled = false;
  objects(meshes.road_segment).disabled = false;
}

static void DisableEditorOnlyMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  objects(meshes.walkway_segment).disabled = true;
  objects(meshes.road_segment).disabled = true;
}

static void HandleFrameEnd(Tachyon* tachyon, State& state) {
  state.allow_frame_step = false;
}

void metro::Init(Tachyon* tachyon, State& state) {
  World::Init(tachyon, state);

  // @todo CameraSystem::Init()
  {
    auto& camera3p = tachyon->scene.camera3p;

    camera3p.azimuth = t_HALF_PI;
    camera3p.altitude = 0.25f;
    camera3p.radius = 10000.f;

    state.target_camera_azimuth = camera3p.azimuth;
  }
}

void metro::Update(Tachyon* tachyon, State& state, const float dt) {
  profile("Game::Update()");

  HandleFrameStart(tachyon, state, dt);

  // @temporary
  tachyon->scene.primary_light_direction = tVec3f(0.5f, -1.f, 0.2f);

  HandleDevHotkeys(tachyon, state);

  if (state.is_editor_open) {
    Debug::Reset(tachyon);
    WorldEditor::Update(tachyon, state);

    StaticEntities::Update(tachyon, state);
    InteractiveEntities::Update(tachyon, state);
    BackgroundVehicles::Update(tachyon, state);

    EnableEditorOnlyMeshes(tachyon, state);

    return;
  }

  if (state.use_frame_stepping && !state.allow_frame_step) {
    CameraSystem::Update(tachyon, state);

    return;
  }

  DisableEditorOnlyMeshes(tachyon, state);

  Debug::Reset(tachyon);

  ControlSystem::Update(tachyon, state);
  StaticEntities::Update(tachyon, state);
  InteractiveEntities::Update(tachyon, state);
  BackgroundVehicles::Update(tachyon, state);
  PlayerVehicle::Update(tachyon, state);
  Player::Update(tachyon, state);
  CameraSystem::Update(tachyon, state);

  HandleFrameEnd(tachyon, state);
}