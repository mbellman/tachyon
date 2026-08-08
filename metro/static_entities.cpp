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

    // @allocation
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

    // @allocation
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

static inline float GetWiderHorizontalScale(const StaticEntity& entity) {
  return std::max(entity.scale.x, entity.scale.z);
}

static inline std::tuple<tVec3f, tVec3f> GetSegmentEdge(const StaticEntity& entity) {
  tVec3f axis = entity.scale.x > entity.scale.z
    ? tVec3f(entity.scale.x, 0, 0)
    : tVec3f(0, 0, entity.scale.z);

  tVec3f axis_offset = entity.rotation.toMatrix4f() * axis;

  return {
    entity.position + axis_offset,
    entity.position + axis_offset * -1.f
  };
}

static void RebuildWalkways(Tachyon* tachyon, State& state) {
  auto& stream = vertex_stream(state.meshes.walkway_stream);

  stream.vertices.clear();
  stream.face_elements.clear();
  stream.buffered = false;

  reset_instances(state.meshes.walkway_plane);

  // Clear any collision planes present on the segment entities.
  // We'll add new ones based on connections between them.
  for (auto& entity : state.entities.walkway_segments) {
    entity.collision_planes.clear();
  }

  for (auto& entity : state.entities.walkway_segments) {
    for (auto& next : state.entities.walkway_segments) {
      if (IsSameEntity(entity, next)) continue;

      float distance = tVec3f::distance(entity.position, next.position);
      tVec3f entity_facing_direction = entity.rotation.getDirection();
      tVec3f next_facing_direction = next.rotation.getDirection();
      tVec3f path_direction = next.position - entity.position;
      float next_dot = tVec3f::dot(path_direction, next_facing_direction);

      if (distance < 15000.f && next_dot > 0.f) {
        float x_scale = (GetWiderHorizontalScale(entity) + GetWiderHorizontalScale(next)) / 2.f;
        float z_scale = distance / 2.f;

        Debug::ShowDebugVector(tachyon, entity.position, entity_facing_direction * 2000.f, tVec3f(1.f));
        Debug::ShowDebugVector(tachyon, next.position, next_facing_direction * 2000.f, tVec3f(1.f));

        float direction_dot = tVec3f::dot(
          entity_facing_direction,
          next.rotation.getLeftDirection()
        );

        // Determine the four corners of the plane between the segments
        auto [A, B] = GetSegmentEdge(entity);
        auto [C, D] = GetSegmentEdge(next);

        // Subdivide the plane for a smoother gradient
        for_range(1, 3) {
          float a1 = float(i - 1) / 3.f;
          float a2 = float(i) / 3.f;

          // Get the four corners of the subdivision
          tVec3f p1 = tVec3f::lerp(A, C, a1);
          tVec3f p2 = tVec3f::lerp(B, D, a1);

          tVec3f p3 = tVec3f::lerp(A, C, a2);
          tVec3f p4 = tVec3f::lerp(B, D, a2);

          if (i > 1) {
            tVec3f shift = (p2 - p1) * direction_dot * 0.2f;

            p1 += shift;
            p2 += shift;
          }

          if (i < 3) {
            tVec3f shift = (p4 - p3) * direction_dot * 0.2f;

            p3 += shift;
            p4 += shift;
          }

          stream.vertices.push_back({
            .position = p1,
            // @temporary
            .normal = tVec3f(0, 1.f, 0)
          });

          stream.vertices.push_back({
            .position = p2,
            // @temporary
            .normal = tVec3f(0, 1.f, 0)
          });

          stream.vertices.push_back({
            .position = p3,
            // @temporary
            .normal = tVec3f(0, 1.f, 0)
          });

          stream.vertices.push_back({
            .position = p4,
            // @temporary
            .normal = tVec3f(0, 1.f, 0)
          });

          uint32 vertex_offset = (uint32) stream.vertices.size() - 4;

          stream.face_elements.push_back(vertex_offset);
          stream.face_elements.push_back(vertex_offset + 2);
          stream.face_elements.push_back(vertex_offset + 1);

          stream.face_elements.push_back(vertex_offset + 1);
          stream.face_elements.push_back(vertex_offset + 2);
          stream.face_elements.push_back(vertex_offset + 3);

          // Debug::ShowDebugSphere(tachyon, p1, 200.f);
          // Debug::ShowDebugSphere(tachyon, p2, 200.f);
          // Debug::ShowDebugSphere(tachyon, p3, 200.f);
          // Debug::ShowDebugSphere(tachyon, p4, 200.f);
        }

        // auto collision_plane = Collision::CreateFloorCollisionPlane(plane);

        // @allocation
        // entity.collision_planes.push_back(collision_plane);
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