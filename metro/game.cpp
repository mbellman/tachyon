#include "metro/game.h"
#include "engine/tachyon_console.h"

void metro::Init(Tachyon* tachyon, State& state) {
  console_log("metro::Init()");

  // @todo
}

void metro::Update(Tachyon* tachyon, State& state, const float dt) {
  // @todo HandleFrameStart()
  {
    state.dt = dt;

    tachyon->scene.scene_time += state.dt;
  }

  console_log("metro::Update()");
  console_log(get_scene_time());

  // @todo
}