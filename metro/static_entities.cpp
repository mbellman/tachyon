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

  static void RebuildWalkways(Tachyon* tachyon, State& state) {
    reset_instances(state.meshes.walkway_plane);

    for (auto& entity : state.entities.walkway_segments) {
      for (auto& next : state.entities.walkway_segments) {
        if (IsSameEntity(entity, next)) continue;

        float distance = tVec3f::distance(entity.position, next.position);

        if (distance < 10000.f) {
          auto& plane = use_instance(state.meshes.walkway_plane);

          plane.position = (entity.position + next.position) / 2.f;
          plane.scale = tVec3f(2000.f, 1.f, 2000.f);
          plane.color = tVec3f(1.f);

          commit(plane);
        }
      }
    }
  }

// ---------------------------

template<typename Entity>
static void HandleLifeCycle(Tachyon* tachyon, State& state, std::vector<StaticEntity>& entities) {
  int32 index = 0;

  for_reversed(entities) {
    auto& entity = entities[i];

    if (entity.needs_deletion) {
      Entity::Remove(tachyon, state, i);

      // Swap-and-pop to mimic the way the entity objects are managed,
      // which also does an effective swap-and-pop on object deletion.
      // This also avoids the need to update any other entities, e.g.
      // the entity taking the deleted entity's place.
      if (i < (int32) entities.size() - 1) {
        std::swap(entities[i], entities.back());
      }

      entities.pop_back();
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

  // If any walkway segments are updated, rebuild all walkway networks
  // @todo needs to be upon deletion as well
  {
    for (auto& entity : state.entities.walkway_segments) {
      if (entity.needs_update) {
        RebuildWalkways(tachyon, state);

        break;
      }
    }
  }

  HandleLifeCycle<Platforms>(tachyon, state, state.entities.platforms);
  HandleLifeCycle<Ramps>(tachyon, state, state.entities.ramps);
  HandleLifeCycle<WalkwaySegments>(tachyon, state, state.entities.walkway_segments);
}