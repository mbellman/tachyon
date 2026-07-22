#include "engine/tachyon.h"

#include "metro/game.h"
#include "metro/background_bicycles.h"
#include "metro/camera_system.h"
#include "metro/control_system.h"
#include "metro/player_bicycle.h"
#include "metro/world_init.h"

using namespace metro;

static void HandleFrameStart(Tachyon* tachyon, State& state, const float dt) {
  state.dt = dt;

  tachyon->scene.scene_time += state.dt;
}

static void HandleFrameEnd(Tachyon* tachyon, State& state) {
  if (did_press_key(tKey::SPACE)) {
    tachyon->show_timing_profile = !tachyon->show_timing_profile;
  }
}

void metro::Init(Tachyon* tachyon, State& state) {
  World::Init(tachyon, state);

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

  ControlSystem::Update(tachyon, state);
  BackgroundBicycles::Update(tachyon, state);
  PlayerBicycle::Update(tachyon, state);
  CameraSystem::Update(tachyon, state);

  HandleFrameEnd(tachyon, state);
}