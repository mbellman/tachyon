#include "metro/game.h"
#include "metro/world_init.h"
#include "metro/world_update.h"

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

  World::Update(tachyon, state);
}