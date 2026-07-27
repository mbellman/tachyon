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

static void HandleFrameEnd(Tachyon* tachyon, State& state) {
  if (did_press_key(tKey::SPACE)) {
    tachyon->show_timing_profile = !tachyon->show_timing_profile;
  }

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

  // @temporary
  {
    if (did_press_key(GAMEPAD_O)) {
      state.use_slow_motion = !state.use_slow_motion;
    }

    if (did_press_key(GAMEPAD_TRIANGLE)) {
      state.use_frame_stepping = !state.use_frame_stepping;
    }

    if (state.use_frame_stepping && did_press_key(tKey::ARROW_RIGHT)) {
      state.allow_frame_step = true;
    }
  }

  CameraSystem::Update(tachyon, state);

  if (state.use_frame_stepping && !state.allow_frame_step) {
    return;
  }

  Debug::HandleFrameStart(tachyon, state);

  ControlSystem::Update(tachyon, state);
  StaticEntities::Update(tachyon, state);
  InteractiveEntities::Update(tachyon, state);
  BackgroundBicycles::Update(tachyon, state);
  PlayerBicycle::Update(tachyon, state);

  HandleFrameEnd(tachyon, state);
}