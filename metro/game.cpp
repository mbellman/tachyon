#include "engine/tachyon.h"

#include "metro/game.h"
#include "metro/background_bicycles.h"
#include "metro/camera_system.h"
#include "metro/collision.h"
#include "metro/control_system.h"
#include "metro/debug.h"
#include "metro/interactive_entities.h"
#include "metro/player_bicycle.h"
#include "metro/static_entities.h"
#include "metro/world_init.h"
#include "metro/utilities.h"

using namespace metro;

const static auto GAMEPAD_X = tKey::CONTROLLER_A;
const static auto GAMEPAD_O = tKey::CONTROLLER_B;
const static auto GAMEPAD_SQUARE = tKey::CONTROLLER_X;
const static auto GAMEPAD_TRIANGLE = tKey::CONTROLLER_Y;

static void HandleFrameStart(Tachyon* tachyon, State& state, const float dt) {
  if (state.use_slow_motion) {
    state.dt = dt * 0.25f;
  } else {
    state.dt = dt;
  }

  tachyon->scene.scene_time += state.dt;
}

static void HandleDevHotkeys(Tachyon* tachyon, State& state) {
  // Toggle function timings
  if (did_press_key(tKey::SPACE)) {
    tachyon->show_timing_profile = !tachyon->show_timing_profile;
  }

  // Toggle slow motion
  if (did_press_key(GAMEPAD_O)) {
    state.use_slow_motion = !state.use_slow_motion;
  }

  // Toggle frame-by-frame
  if (did_press_key(GAMEPAD_TRIANGLE)) {
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
    auto* active_bike = GetActiveBicycle(state);

    if (active_bike != nullptr) {
      // @temporary
      // @todo create a method for resetting motion/rotation etc.
      active_bike->position = tVec3f(0, -2220.f, -10000.f);
      active_bike->visual_position = active_bike->position;
      active_bike->pedal_speed = 0.f;
      active_bike->speed = 0.f;
      active_bike->pitch = 0.f;
      active_bike->facing_direction = tVec3f(0, 0, -1.f);
      active_bike->drifting_factor = 0.f;
      active_bike->steering_angle = 0.f;
      active_bike->leaning_angle = 0.f;

      active_bike->flat_rotation =
        Quaternion::FromDirection(active_bike->facing_direction, tVec3f(0, 1.f, 0)) *
        Quaternion::fromAxisAngle(tVec3f(0, 0, 1.f), active_bike->leaning_angle);

      active_bike->directional_rotation = active_bike->flat_rotation;
      active_bike->visual_rotation = active_bike->flat_rotation;
    }
  }
}

static void HandleFrameEnd(Tachyon* tachyon, State& state) {
  state.allow_frame_step = false;
}

void metro::Init(Tachyon* tachyon, State& state) {
  World::Init(tachyon, state);
  StaticEntities::Init(tachyon, state);
  InteractiveEntities::Init(tachyon, state);

  // @todo CameraSystem::Init()
  {
    auto& camera3p = tachyon->scene.camera3p;

    camera3p.altitude = 0.25f;
    camera3p.radius = 10000.f;

    camera3p.azimuth = t_HALF_PI;
  }
}

void metro::Update(Tachyon* tachyon, State& state, const float dt) {
  profile("Game::Update()");

  HandleFrameStart(tachyon, state, dt);

  // @temporary
  tachyon->scene.primary_light_direction = tVec3f(0.5f, -1.f, 0.2f);

  HandleDevHotkeys(tachyon, state);

  if (state.use_frame_stepping && !state.allow_frame_step) {
    CameraSystem::Update(tachyon, state);

    return;
  }

  Debug::HandleFrameStart(tachyon, state);

  ControlSystem::Update(tachyon, state);
  StaticEntities::Update(tachyon, state);
  InteractiveEntities::Update(tachyon, state);
  BackgroundBicycles::Update(tachyon, state);
  PlayerBicycle::Update(tachyon, state);
  CameraSystem::Update(tachyon, state);

  HandleFrameEnd(tachyon, state);
}