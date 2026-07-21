#include "metro/game.h"
#include "metro/world_init.h"
#include "metro/world_update.h"

using namespace metro;

void metro::Init(Tachyon* tachyon, State& state) {
  World::Init(tachyon, state);
}

void metro::Update(Tachyon* tachyon, State& state, const float dt) {
  // @todo HandleFrameStart()
  {
    state.dt = dt;

    tachyon->scene.scene_time += state.dt;
  }

  World::Update(tachyon, state);
}