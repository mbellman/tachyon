#include "metro/static_entities.h"

using namespace metro;

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

        tVec3f unit_path_direction = path_direction / distance;

        // Determine the four corners of the plane between the segments
        auto [A, B] = GetSegmentEdge(entity);
        auto [C, D] = GetSegmentEdge(next);

        float edge_AC_factor = tVec3f::distance(A, C) / distance;
        float edge_BD_factor = tVec3f::distance(B, D) / distance;

        const int total_slices = 5;
        uint32 vertex_offset = (uint32) stream.vertices.size();

        // Create vertices to form subdivided slices of the plane
        for_range(0, total_slices) {
          float a = float(i) / (float) total_slices;

          tVec3f p1 = tVec3f::lerp(A, C, a);
          tVec3f p2 = tVec3f::lerp(B, D, a);

          if (i > 0 && i < total_slices) {
            float shift_factor = 0.25f * sinf(a * t_PI);

            tVec3f p1_shift = (p2 - p1) * direction_dot * shift_factor * edge_AC_factor;
            tVec3f p2_shift = (p2 - p1) * direction_dot * shift_factor * edge_BD_factor;

            p1 += p1_shift;
            p2 += p2_shift;
          }

          tVec3f normal = tVec3f::cross(
            unit_path_direction,
            (p2 - p1).unit()
          );

          stream.vertices.push_back({
            .position = p1,
            .normal = normal
          });

          stream.vertices.push_back({
            .position = p2,
            .normal = normal
          });
        }

        // Create face elements + collision for the subdivided plane slices
        for_range(1, total_slices) {
          uint32 offset = vertex_offset + (i - 1) * 2;

          // Triangle 1; 0 2 1
          stream.face_elements.push_back(offset);
          stream.face_elements.push_back(offset + 2);
          stream.face_elements.push_back(offset + 1);

          // Triangle 2; 1 2 3
          stream.face_elements.push_back(offset + 1);
          stream.face_elements.push_back(offset + 2);
          stream.face_elements.push_back(offset + 3);

          // Triangle 1 collision
          {
            CollisionPlane plane;
            plane.p1 = stream.vertices[offset].position;
            plane.p2 = stream.vertices[offset + 2].position;
            plane.p3 = stream.vertices[offset + 1].position;
            plane.p4 = stream.vertices[offset].position;

            Collision::PadCollisionPlane(plane, 300.f);
            Collision::PrepareCollisionPlane(plane);

            // @allocation
            entity.collision_planes.push_back(plane);
          }

          // Triangle 2 collision
          {
            CollisionPlane plane;
            plane.p1 = stream.vertices[offset + 1].position;
            plane.p2 = stream.vertices[offset + 2].position;
            plane.p3 = stream.vertices[offset + 3].position;
            plane.p4 = stream.vertices[offset + 1].position;

            Collision::PadCollisionPlane(plane, 300.f);
            Collision::PrepareCollisionPlane(plane);

            // @allocation
            entity.collision_planes.push_back(plane);
          }
        }
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