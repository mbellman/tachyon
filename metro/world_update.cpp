#include "engine/tachyon.h"

#include "metro/world_update.h"

using namespace metro;

void World::Update(Tachyon* tachyon, State& state) {
  // @temporary
  tachyon->scene.primary_light_direction = tVec3f(0.5f, -1.f, 0.2f);

  // @temporary
  {
    auto& bicycle = objects(state.meshes.bicycle)[0];

    bicycle.rotation = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), get_scene_time());

    commit(bicycle);
  }
}