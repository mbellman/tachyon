#include "engine/tachyon.h"

#include "metro/world_update.h"
#include "metro/background_bicycles.h"
#include "metro/player_bicycle.h"

using namespace metro;

void World::Update(Tachyon* tachyon, State& state) {
  // @temporary
  tachyon->scene.primary_light_direction = tVec3f(0.5f, -1.f, 0.2f);

  BackgroundBicycles::Update(tachyon, state);
  PlayerBicycle::Update(tachyon, state);
}