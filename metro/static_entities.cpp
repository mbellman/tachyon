#include "metro/static_entities.h"
#include "metro/collision.h"

using namespace metro;

#define for_reversed(array)\
  for (int32 i = (int32) array.size() - 1; i >= 0; i--)

#define OnInit() static void Init(Tachyon* tachyon, State& state)
#define OnUpdate() static void Update(Tachyon* tachyon, State& state, StaticEntity& entity, int32 index)
#define OnRemove() static void Remove(Tachyon* tachyon, State& state, int32 index)

static void Sync(tObject& object, const StaticEntity& entity) {
  object.position = entity.position;
  object.scale = entity.scale;
  object.rotation = entity.rotation;
  object.color = entity.color;
}

// ---------
// Platforms
// ---------

struct Platforms {
  OnInit() {
    create(state.meshes.platform);
  }

  OnUpdate() {
    auto& platform = objects(state.meshes.platform)[index];

    Sync(platform, entity);

    commit(platform);

    auto plane = Collision::CreateFloorCollisionPlane(platform);

    entity.collision_planes.clear();
    entity.collision_planes.push_back(plane);
    entity.needs_update = false;
  }

  OnRemove() {
    auto& object = objects(state.meshes.platform)[index];

    remove_object(object);
  }
};

// -----
// Ramps
// -----

struct Ramps {
  OnInit() {
    create(state.meshes.ramp);
  }

  OnUpdate() {
    auto& ramp = objects(state.meshes.ramp)[index];

    Sync(ramp, entity);

    commit(ramp);

    auto plane = Collision::CreateSlopeCollisionPlane(ramp);

    entity.collision_planes.clear();
    entity.collision_planes.push_back(plane);
    entity.needs_update = false;
  }

  OnRemove() {
    auto& object = objects(state.meshes.ramp)[index];

    remove_object(object);
  }
};

// ----------------
// Walkway Segments
// ----------------

struct WalkwaySegments {
  OnInit() {
    create(state.meshes.walkway_segment);
  }

  OnUpdate() {
    auto& segment = objects(state.meshes.walkway_segment)[index];

    Sync(segment, entity);

    commit(segment);

    entity.needs_update = false;
  }

  OnRemove() {
    auto& object = objects(state.meshes.walkway_segment)[index];

    remove_object(object);
  }
};

// ---------------------------

template<typename Entity>
static void HandleLifeCycle(Tachyon* tachyon, State& state, std::vector<StaticEntity>& entities) {
  int32 index = 0;

  for_reversed(entities) {
    auto& entity = entities[i];

    if (entity.needs_deletion) {
      Entity::Remove(tachyon, state, i);

      entities.erase(entities.begin() + i);
    }
  }

  for (auto& entity : entities) {
    if (entity.needs_init) {
      Entity::Init(tachyon, state);

      entity.needs_init = false;
    }

    int32 current_index = index++;

    if (entity.needs_update) {
      Entity::Update(tachyon, state, entity, current_index);
    }
  }
}

void StaticEntities::Update(Tachyon* tachyon, State& state) {
  profile("StaticEntities::Update()");

  HandleLifeCycle<Platforms>(tachyon, state, state.entities.platforms);
  HandleLifeCycle<Ramps>(tachyon, state, state.entities.ramps);
  HandleLifeCycle<WalkwaySegments>(tachyon, state, state.entities.walkway_segments);
}