#include "metro/static_entities.h"
#include "metro/collision.h"

using namespace metro;

static void Sync(tObject& object, const StaticEntity& entity) {
  object.position = entity.position;
  object.scale = entity.scale;
  object.rotation = entity.rotation;
  object.color = entity.color;
}

// -----
// Ramps
// -----

namespace Ramps {
  void Init(Tachyon* tachyon, State& state) {
    for (auto& entity : state.ramps) {
      auto& ramp = create(state.meshes.ramp);

      Sync(ramp, entity);

      commit(ramp);

      Collision::AddSlopeCollision(state, ramp);
    }
  }

  void Update(Tachyon* tachyon, State& state) {
    // @todo
  }
};

// --------
// Walkways
// --------

namespace Walkways {
  void Init(Tachyon* tachyon, State& state) {
    for (auto& entity : state.walkway_segments) {
      auto& segment = create(state.meshes.walkway_segment);

      Sync(segment, entity);

      commit(segment);
    }
  }

  void Update(Tachyon* tachyon, State& state) {
    int32 index = 0;

    for (auto& entity : state.walkway_segments) {
      auto& segment = objects(state.meshes.walkway_segment)[index++];

      if (!entity.modified) continue;

      Sync(segment, entity);

      commit(segment);

      entity.modified = false;
    }
  }
};

// ---------------------------

void StaticEntities::Init(Tachyon* tachyon, State& state) {
  Ramps::Init(tachyon, state);
  Walkways::Init(tachyon, state);
}

void StaticEntities::Update(Tachyon* tachyon, State& state) {
  profile("StaticEntities::Update()");

  Ramps::Update(tachyon, state);
  Walkways::Update(tachyon, state);
}