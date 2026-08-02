#include "metro/static_entities.h"
#include "metro/collision.h"

using namespace metro;

static void Sync(tObject& object, const StaticEntity& entity) {
  object.position = entity.position;
  object.scale = entity.scale;
  object.rotation = entity.rotation;
  object.color = entity.color;
}

// ---------
// Platforms
// ---------

namespace Platforms {
  void Init(Tachyon* tachyon, State& state) {
    for (auto& entity : state.platforms) {
      create(state.meshes.platform);
    }
  }

  void Update(Tachyon* tachyon, State& state) {
    int32 index = 0;

    for (auto& entity : state.platforms) {
      auto& platform = objects(state.meshes.platform)[index++];

      if (!entity.needs_update) continue;

      Sync(platform, entity);

      commit(platform);

      auto plane = Collision::CreateFloorCollisionPlane(platform);

      entity.collision_planes.clear();
      entity.collision_planes.push_back(plane);
      entity.needs_update = false;
    }
  }
};

// -----
// Ramps
// -----

namespace Ramps {
  void Init(Tachyon* tachyon, State& state) {
    for (auto& entity : state.ramps) {
      create(state.meshes.ramp);
    }
  }

  void Update(Tachyon* tachyon, State& state) {
    int32 index = 0;

    for (auto& entity : state.ramps) {
      auto& ramp = objects(state.meshes.ramp)[index++];

      if (!entity.needs_update) continue;

      Sync(ramp, entity);

      commit(ramp);

      auto plane = Collision::CreateSlopeCollisionPlane(ramp);

      entity.collision_planes.clear();
      entity.collision_planes.push_back(plane);
      entity.needs_update = false;
    }
  }
};

// --------
// Walkways
// --------

namespace Walkways {
  void Init(Tachyon* tachyon, State& state) {
    for (auto& entity : state.walkway_segments) {
      create(state.meshes.walkway_segment);
    }
  }

  void Update(Tachyon* tachyon, State& state) {
    int32 index = 0;

    for (auto& entity : state.walkway_segments) {
      auto& segment = objects(state.meshes.walkway_segment)[index++];

      if (!entity.needs_update) continue;

      Sync(segment, entity);

      commit(segment);

      entity.needs_update = false;
    }
  }
};

// ---------------------------

void StaticEntities::Init(Tachyon* tachyon, State& state) {
  Platforms::Init(tachyon, state);
  Ramps::Init(tachyon, state);
  Walkways::Init(tachyon, state);
}

void StaticEntities::Update(Tachyon* tachyon, State& state) {
  profile("StaticEntities::Update()");

  Platforms::Update(tachyon, state);
  Ramps::Update(tachyon, state);
  Walkways::Update(tachyon, state);
}