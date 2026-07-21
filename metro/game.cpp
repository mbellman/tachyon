#include "metro/game.h"
#include "metro/background_bicycles.h"
#include "metro/player_bicycle.h"
#include "metro/world_init.h"

using namespace metro;

static void HandleFrameStart(Tachyon* tachyon, State& state, const float dt) {
  state.dt = dt;

  tachyon->scene.scene_time += state.dt;
}

void metro::Init(Tachyon* tachyon, State& state) {
  World::Init(tachyon, state);
}

void metro::Update(Tachyon* tachyon, State& state, const float dt) {
  HandleFrameStart(tachyon, state, dt);

  // @temporary
  tachyon->scene.primary_light_direction = tVec3f(0.5f, -1.f, 0.2f);

  BackgroundBicycles::Update(tachyon, state);
  PlayerBicycle::Update(tachyon, state);
}