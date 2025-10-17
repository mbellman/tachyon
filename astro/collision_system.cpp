#include "astro/collision_system.h"

using namespace astro;

constexpr static float player_radius = 600.f;

static std::vector<tVec3f> small_bridge_points = {
  tVec3f(-1.f, 0, 0.65f),
  tVec3f(1.f, 0, 0.65f),
  tVec3f(1.f, 0, -0.65f),
  tVec3f(-1.f, 0, -0.65f)
};

static std::vector<tVec3f> river_log_points = {
  tVec3f(-0.2f, 0, 1.f),
  tVec3f(0.2f, 0, 1.f),
  tVec3f(0.2f, 0, -1.f),
  tVec3f(-0.2f, 0, -1.f)
};

// @temporary
// @todo replace with proper line/plane collision checks
static inline bool IsPointInsideEdge(const tVec3f& point, const tVec3f& e1, const tVec3f& e2) {
  return (
    (e2.x - e1.x) * (point.z - e1.z) -
    (point.x - e1.x) * (e2.z - e1.z)
  ) > 0.f;
}

static inline tVec3f GetOversteppedEdge(const tVec3f& point, const Plane& plane) {
  if (!IsPointInsideEdge(point, plane.p2, plane.p1)) {
    return plane.p2 - plane.p1;
  }
  else if (!IsPointInsideEdge(point, plane.p3, plane.p2)) {
    return plane.p3 - plane.p2;
  }
  else if (!IsPointInsideEdge(point, plane.p4, plane.p3)) {
    return plane.p4 - plane.p3;
  }
  else if (!IsPointInsideEdge(point, plane.p1, plane.p4)) {
    return plane.p1 - plane.p4;
  }

  // We .unit() the returned value, so ensure its magnitude is not zero
  // in the fallback case, which would cause divide-by-zero issues
  return tVec3f(1.f, 0, 0);
}

static inline bool IsPointWithRadiusOnPlane(const tVec3f& point, const float radius, const Plane& plane) {
  tVec3f plane_midpoint = (plane.p1 + plane.p2 + plane.p3 + plane.p4) / 4.f;
  tVec3f plane_to_point = point.xz() - plane_midpoint.xz();
  tVec3f adjusted_point = point - plane_to_point.unit() * radius;

  return CollisionSystem::IsPointOnPlane(adjusted_point, plane);
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

static void HandleFlatGroundCollisions(Tachyon* tachyon, State& state) {
  state.player_position.y = state.water_level + 1500.f;

  for (auto& ground : objects(state.meshes.flat_ground)) {
    // @todo factor
    Plane ground_plane = {
      tVec3f(-1.f, 0, 1.f) * ground.scale,
      tVec3f(1.f, 0, 1.f) * ground.scale,
      tVec3f(1.f, 0, -1.f) * ground.scale,
      tVec3f(-1.f, 0, -1.f) * ground.scale
    };

    tMat4f r = ground.rotation.toMatrix4f();

    ground_plane.p1 = ground.position + r * ground_plane.p1;
    ground_plane.p2 = ground.position + r * ground_plane.p2;
    ground_plane.p3 = ground.position + r * ground_plane.p3;
    ground_plane.p4 = ground.position + r * ground_plane.p4;

    if (IsPointWithRadiusOnPlane(state.player_position, 400.f, ground_plane)) {
      // @todo set to height of ground
      state.player_position.y = 0.f;
      state.last_solid_ground_position = state.player_position;
      state.is_on_solid_ground = true;
      state.last_plane_walked_on = ground_plane;

      break;
    }
  }
}

static void HandleBridgeCollisions(Tachyon* tachyon, State& state) {
  for_entities(state.small_stone_bridges) {
    auto& bridge = state.small_stone_bridges[i];

    // @todo prevent the player from walking over the bridge area when its visible scale = 0
    if (bridge.visible_scale == tVec3f(0.f)) {
      continue;
    }

    // @temporary
    // @todo factor
    // @todo properly handle collisions for different parts of the bridge
    Plane bridge_plane = {
      small_bridge_points[0] * bridge.scale,
      small_bridge_points[1] * bridge.scale,
      small_bridge_points[2] * bridge.scale,
      small_bridge_points[3] * bridge.scale
    };

    // @todo @optimize this need not be computed every frame
    tMat4f r = bridge.orientation.toMatrix4f();

    bridge_plane.p1 = bridge.position + r * bridge_plane.p1;
    bridge_plane.p2 = bridge.position + r * bridge_plane.p2;
    bridge_plane.p3 = bridge.position + r * bridge_plane.p3;
    bridge_plane.p4 = bridge.position + r * bridge_plane.p4;

    if (CollisionSystem::IsPointOnPlane(state.player_position.xz(), bridge_plane)) {
      // Figure out how far along the bridge the player is,
      // and set their height accordingly
      tVec3f bridge_to_player = state.player_position - bridge.position;
      tVec3f player_position_in_bridge_space = r.inverse() * bridge_to_player;
      float midpoint_ratio = 1.f - abs(player_position_in_bridge_space.x) / bridge.scale.x;
      float floor_height = Tachyon_EaseOutQuad(midpoint_ratio);

      state.player_position.y = floor_height * bridge.scale.y / 2.2f;
      state.last_solid_ground_position = state.player_position;
      state.is_on_solid_ground = true;
      state.last_plane_walked_on = bridge_plane;

      break;
    }
  }
}

static void HandleRiverLogCollisions(Tachyon* tachyon, State& state) {
  for_entities(state.river_logs) {
    auto& entity = state.river_logs[i];
    auto& log = objects(state.meshes.river_log)[i];

    if (entity.visible_scale == tVec3f(0.f)) {
      continue;
    }

    // @todo factor
    // @temporary
    Plane log_plane = {
      river_log_points[0] * entity.scale,
      river_log_points[1] * entity.scale,
      river_log_points[2] * entity.scale,
      river_log_points[3] * entity.scale
    };

    // @todo @optimize this need not be computed every frame
    tMat4f r = log.rotation.toMatrix4f();

    log_plane.p1 = log.position + r * log_plane.p1;
    log_plane.p2 = log.position + r * log_plane.p2;
    log_plane.p3 = log.position + r * log_plane.p3;
    log_plane.p4 = log.position + r * log_plane.p4;

    if (CollisionSystem::IsPointOnPlane(state.player_position.xz(), log_plane)) {
      // @todo define a constant for player height
      state.player_position.y = log.position.y + log.scale.y * 0.2f + 1500.f;
      state.last_solid_ground_position = state.player_position;
      state.is_on_solid_ground = true;
      state.last_plane_walked_on = log_plane;

      break;
    }
  }
}

static void HandleMovementOffSolidGround(Tachyon* tachyon, State& state) {
  tVec3f edge = GetOversteppedEdge(state.player_position, state.last_plane_walked_on);
  float edge_movement_dot = tVec3f::dot(state.player_velocity, edge);
  tVec3f corrected_direction = edge_movement_dot > 0.f ? edge.unit() : edge.invert().unit();
  tVec3f rebound_direction = tVec3f::cross(tVec3f(0, 1.f, 0), edge.unit());

  state.player_position = state.last_solid_ground_position + rebound_direction * 5.f;
  state.player_velocity = corrected_direction * state.player_velocity.magnitude();
}

bool CollisionSystem::IsPointOnPlane(const tVec3f& point, const Plane& plane) {
  bool d1 = IsPointInsideEdge(point, plane.p2, plane.p1);
  bool d2 = IsPointInsideEdge(point, plane.p3, plane.p2);
  bool d3 = IsPointInsideEdge(point, plane.p4, plane.p3);
  bool d4 = IsPointInsideEdge(point, plane.p1, plane.p4);

  return d1 && d2 && d3 && d4;
}

Plane CollisionSystem::GetEntityPlane(const GameEntity& entity) {
  Plane plane;
  plane.p1 = tVec3f(-1.f, 0, 1.f) * entity.scale;
  plane.p2 = tVec3f(1.f, 0, 1.f) * entity.scale;
  plane.p3 = tVec3f(1.f, 0, -1.f) * entity.scale;
  plane.p4 = tVec3f(-1.f, 0, -1.f) * entity.scale;

  tMat4f r = entity.orientation.toMatrix4f();

  plane.p1 = entity.position + r * plane.p1;
  plane.p2 = entity.position + r * plane.p2;
  plane.p3 = entity.position + r * plane.p3;
  plane.p4 = entity.position + r * plane.p4;

  return plane;
}

void CollisionSystem::HandleCollisions(Tachyon* tachyon, State& state) {
  profile("HandleCollisions()");

  for_entities(state.shrubs) {
    auto& entity = state.shrubs[i];

    ResolveSingleRadiusCollision(state, entity.position, entity.visible_scale, 1.5f);
  }

  for_entities(state.oak_trees) {
    auto& entity = state.oak_trees[i];

    ResolveSingleRadiusCollision(state, entity.position, entity.visible_scale, 0.8f);
  }

  for (auto& rock : objects(state.meshes.rock_1)) {
    ResolveSingleRadiusCollision(state, rock.position, rock.scale, 1.f);
  }

  for (auto& ground : objects(state.meshes.ground_1)) {
    ResolveSingleRadiusCollision(state, ground.position, ground.scale, 0.8f);
  }

  state.is_on_solid_ground = false;

  HandleFlatGroundCollisions(tachyon, state);
  HandleBridgeCollisions(tachyon, state);
  HandleRiverLogCollisions(tachyon, state);

  if (!state.is_on_solid_ground) {
    HandleMovementOffSolidGround(tachyon, state);
  }
}