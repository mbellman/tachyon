#include "astro/collision_system.h"

using namespace astro;

constexpr static float player_radius = 600.f;

static std::vector<tVec3f> small_bridge_points = {
  tVec3f(-1.f, 0, 0.65f),
  tVec3f(1.f, 0, 0.65f),
  tVec3f(1.f, 0, -0.65f),
  tVec3f(-1.f, 0, -0.65f)
};

struct Plane {
  tVec3f p1, p2, p3, p4;
};

// @temporary
// @todo replace with proper line/plane collision checks
static inline bool IsPointInsideEdge(const tVec3f& point, const tVec3f& e1, const tVec3f& e2) {
  return (
    (e2.x - e1.x) * (point.z - e1.z) -
    (point.x - e1.x) * (e2.z - e1.z)
  ) > 0.f;
}

// @temporary
// @todo replace with proper line/plane collision checks
static inline bool IsPointOnPlane(const tVec3f& point, const Plane& plane) {
  bool d1 = IsPointInsideEdge(point, plane.p2, plane.p1);
  bool d2 = IsPointInsideEdge(point, plane.p3, plane.p2);
  bool d3 = IsPointInsideEdge(point, plane.p4, plane.p3);
  bool d4 = IsPointInsideEdge(point, plane.p1, plane.p4);

  return d1 && d2 && d3 && d4;
}

static inline void ResolveSingleRadiusCollision(State& state, const tVec3f& position, const tVec3f& scale, float radius_scale) {
  float radius = scale.x > scale.z
    ? radius_scale * scale.x
    : radius_scale * scale.z;

  // Skip small or invisible radii
  if (radius < 0.1f) return;

  float dx = state.player_position.x - position.x;
  float dz = state.player_position.z - position.z;
  float distance = sqrtf(dx*dx + dz*dz) - player_radius;

  // Prevent excessively small or negative distances
  if (distance < 100.f) distance = 100.f;

  if (distance < radius) {
    float ratio = radius / distance;

    // Ensure the ratio is kept small to avoid large
    // instantaneous displacements out of the radius
    if (ratio > 1.1f) ratio = 1.1f;

    state.player_position.x = position.x + dx * ratio;
    state.player_position.z = position.z + dz * ratio;
  }
}

static void HandleBridgeCollisions(Tachyon* tachyon, State& state) {
  auto& player_position = state.player_position;

  // @todo handle gravity
  player_position.y = 0.f;

  for_entities(state.small_stone_bridges) {
    auto& bridge = state.small_stone_bridges[i];

    // @todo prevent the player from walking over the bridge area when its visible scale = 0
    if (bridge.visible_scale == tVec3f(0.f)) {
      continue;
    }

    // @temporary
    // @todo properly handle collisions for different parts of the bridge
    Plane plane = {
      small_bridge_points[0] * bridge.scale,
      small_bridge_points[1] * bridge.scale,
      small_bridge_points[2] * bridge.scale,
      small_bridge_points[3] * bridge.scale
    };

    tMat4f m = bridge.orientation.toMatrix4f();

    plane.p1 = bridge.position + m * plane.p1;
    plane.p2 = bridge.position + m * plane.p2;
    plane.p3 = bridge.position + m * plane.p3;
    plane.p4 = bridge.position + m * plane.p4;

    tVec3f point = state.player_position * tVec3f(1.f, 0, 1.f);

    if (IsPointOnPlane(point, plane)) {
      player_position.y = 1000.f;
    }
  }
}

void CollisionSystem::HandleCollisions(Tachyon* tachyon, State& state) {
  for_entities(state.shrubs) {
    auto& entity = state.shrubs[i];

    ResolveSingleRadiusCollision(state, entity.position, entity.visible_scale, 1.5f);
  }

  for_entities(state.oak_trees) {
    auto& entity = state.oak_trees[i];

    ResolveSingleRadiusCollision(state, entity.position, entity.visible_scale, 1.5f);
  }

  for (auto& rock : objects(state.meshes.rock_1)) {
    ResolveSingleRadiusCollision(state, rock.position, rock.scale, 1.f);
  }

  HandleBridgeCollisions(tachyon, state);
}