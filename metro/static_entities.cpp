#include "metro/static_entities.h"
#include "metro/collision.h"

using namespace metro;

// -----
// Ramps
// -----

static void InitRamps(Tachyon* tachyon, State& state) {
  for (auto& entity : state.ramps) {
    auto& ramp = create(state.meshes.ramp);

    ramp.position = entity.position;
    ramp.scale = entity.scale;
    ramp.rotation = entity.rotation;
    ramp.color = entity.color;

    commit(ramp);

    Collision::AddSlopeCollision(state, ramp);
  }
}

static void UpdateRamps(Tachyon* tachyon, State& state) {
  // @todo
}

// ---------------------------

void StaticEntities::Init(Tachyon* tachyon, State& state) {
  InitRamps(tachyon, state);
}

void StaticEntities::Update(Tachyon* tachyon, State& state) {
  UpdateRamps(tachyon, state);
}