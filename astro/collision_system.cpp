#include "astro/collision_system.h"
#include "astro/player_character.h"
#include "astro/entity_behaviors/behavior.h"

using namespace astro;

constexpr static float PLAYER_RADIUS = 600.f;
constexpr static float PLAYER_HEIGHT = 1500.f;

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

// @todo remove radius_scale argument and just premultiply scale
static inline void ResolveSingleRadiusCollision(State& state, const tVec3f& position, const tVec3f& scale, float radius_scale) {
  float radius = scale.x > scale.z
    ? radius_scale * scale.x
    : radius_scale * scale.z;

  // Skip small or invisible radii
  if (radius < 0.1f) return;

  float dx = state.player_position.x - position.x;
  float dz = state.player_position.z - position.z;
  float distance = sqrtf(dx*dx + dz*dz) - PLAYER_RADIUS;

  // Prevent excessively small or negative distances
  if (distance < 100.f) distance = 100.f;

  if (distance < radius) {
    float ratio = radius / distance;

    // Ensure the ratio is kept small to avoid large
    // instantaneous displacements out of the radius
    if (ratio > 1.1f) ratio = 1.1f;

    state.player_position.x = position.x + dx * ratio;
    state.player_position.z = position.z + dz * ratio;

    state.did_resolve_radius_collision = true;
  }
}

static inline void ResolveSoftRadiusCollision(State& state, const tVec3f& position, const float radius) {
  // Skip small or invisible radii
  if (radius < 0.1f) return;

  float dx = state.player_position.x - position.x;
  float dz = state.player_position.z - position.z;
  float distance = sqrtf(dx*dx + dz*dz) - PLAYER_RADIUS;

  // Prevent excessively small or negative distances
  if (distance < 100.f) distance = 100.f;

  if (distance < radius) {
    float ratio = distance / radius;

    state.player_position.x += dx * state.dt * 10.f * (1.f - ratio);
    state.player_position.z += dz * state.dt * 10.f * (1.f - ratio);

    state.did_resolve_radius_collision = true;
  }
}

static bool ResolveClippingIntoPlane(State& state, const Plane& plane) {
  tVec3f player_xz = state.player_position.xz();

  if (CollisionSystem::IsPointOnPlane(player_xz, plane)) {
    if (state.did_resolve_radius_collision) {
      // When we've resolved an earlier radius collision, prevent
      // clipping by simply resetting the player position to the
      // last solid ground position as a failsafe for both collisions.
      // Otherwise, plane collision resolution can cause us to
      // pass through radius collision bounds.
      state.player_position = state.last_solid_ground_position;
    } else {
      float player_speed = state.player_velocity.magnitude();
      tVec3f unit_velocity = state.player_velocity / player_speed;
      tVec3f crossed_edge = GetOversteppedEdge(state.last_solid_ground_position, plane).unit();
      float edge_dot = tVec3f::dot(unit_velocity, crossed_edge);
      tVec3f corrected_direction = edge_dot > 0.f ? crossed_edge : crossed_edge.invert();
      float corrected_speed = 3.f * player_speed * abs(edge_dot) * state.dt;
      tVec3f rebound_direction = tVec3f::cross(crossed_edge, tVec3f(0, 1.f, 0));

      // Reset position
      state.player_position = state.last_solid_ground_position;
      // Add rebound to mitigate any accidental clipping in
      state.player_position += rebound_direction * 0.1f;
      // Slide along the egde to conserve movement
      state.player_position += corrected_direction * corrected_speed;
    }

    // Resolved collision
    return true;
  }

  // No collision to resolve
  return false;
}

static void AllowPlayerMovement(State& state, const float y, const Plane& plane) {
  state.current_ground_y = y;
  state.last_solid_ground_position = state.player_position;
  state.last_solid_ground_position.y = y;
  state.is_on_solid_ground = true;
  state.last_plane_walked_on = plane;
}

static void HandleGateCollisions(Tachyon* tachyon, State& state) {
  const tVec3f scale_factor = tVec3f(0.4f, 0, 1.4f);

  tVec3f player_xz = state.player_position.xz();
  float player_speed = state.player_velocity.magnitude();

  // @allocation
  // @todo come up with a better mechanism for this
  std::vector<Plane> collision_planes;

  for (auto& entity : state.gates) {
    bool is_open = (
      entity.game_activation_time > -1.f &&
      time_since(entity.game_activation_time) > 1.f &&
      state.astro_time >= entity.astro_activation_time
    );

    // @todo come up with a better mechanism for this
    collision_planes.clear();

    if (is_open) {
      // @todo cleanup
      tVec3f wall_center = entity.visible_scale * tVec3f(0, 0, 0.7f);
      tVec3f wall_scale = entity.visible_scale * tVec3f(1.f, 1.f, 0.46f);
      tVec3f wall_center_offset = entity.orientation.toMatrix4f() * wall_center;

      tVec3f left_plane_position = entity.position + wall_center_offset;
      tVec3f left_plane_scale = wall_scale;

      tVec3f right_plane_position = entity.position - wall_center_offset;
      tVec3f right_plane_scale = wall_scale;

      auto plane1 = CollisionSystem::CreatePlane(left_plane_position, left_plane_scale, entity.orientation);
      auto plane2 = CollisionSystem::CreatePlane(right_plane_position, right_plane_scale, entity.orientation);

      collision_planes.push_back(plane1);
      collision_planes.push_back(plane2);
    } else {
      auto plane = CollisionSystem::CreatePlane(entity.position, entity.visible_scale, entity.orientation);

      collision_planes.push_back(plane);
    }

    for (auto& plane : collision_planes) {
      if (ResolveClippingIntoPlane(state, plane)) {
        return;
      }
    }
  }
}

static void HandleWoodenFenceCollisions(Tachyon* tachyon, State& state) {
  for (auto& entity : state.wooden_fences) {
    if (state.astro_time < entity.astro_start_time) continue;

    auto fence_plane = CollisionSystem::CreatePlane(entity.position, entity.visible_scale, entity.orientation);

    ResolveClippingIntoPlane(state, fence_plane);
  }
}

static void HandleHouseCollisions(Tachyon* tachyon, State& state) {
  for (auto& entity : state.houses) {
    // @todo
    // if (state.astro_time < entity.astro_start_time) continue;

    auto house_plane = CollisionSystem::CreatePlane(entity.position, entity.visible_scale * 1.2f, entity.orientation);

    ResolveClippingIntoPlane(state, house_plane);
  }
}

static void HandleFlatGroundCollisions(Tachyon* tachyon, State& state) {
  for (auto& ground : objects(state.meshes.flat_ground)) {
    auto ground_plane = CollisionSystem::CreatePlane(ground.position, ground.scale, ground.rotation);

    if (IsPointWithRadiusOnPlane(state.player_position, 400.f, ground_plane)) {
      // @todo use actual ground object height
      float ground_y = 0.f;

      AllowPlayerMovement(state, ground_y, ground_plane);

      break;
    }
  }
}

static void HandleSmallStoneBridgeCollisions(Tachyon* tachyon, State& state) {
  for (auto& entity : state.small_stone_bridges) {
    if (entity.visible_scale == tVec3f(0.f)) continue;

    tMat4f entity_rotation_matrix = entity.orientation.toMatrix4f();

    // Edge collision
    tVec3f edge_1_center = entity.position + entity_rotation_matrix * (tVec3f(0, 0, 0.5f) * entity.scale);
    tVec3f edge_2_center = entity.position + entity_rotation_matrix * (tVec3f(0, 0, -0.5f) * entity.scale);
    tVec3f edge_scale = entity.scale * tVec3f(1.1f, 0, 0.2f);

    auto edge_1_plane = CollisionSystem::CreatePlane(edge_1_center, edge_scale, entity.orientation);
    auto edge_2_plane = CollisionSystem::CreatePlane(edge_2_center, edge_scale, entity.orientation);

    ResolveClippingIntoPlane(state, edge_1_plane);
    ResolveClippingIntoPlane(state, edge_2_plane);

    // Bridge walkway collision
    auto bridge_plane = CollisionSystem::CreatePlane(entity.position, entity.scale * tVec3f(1.f, 0, 0.5f), entity.orientation);

    if (CollisionSystem::IsPointOnPlane(state.player_position.xz(), bridge_plane)) {
      // Figure out how far along the bridge the player is,
      // and set their height accordingly
      tVec3f bridge_to_player = state.player_position - entity.position;
      tVec3f player_position_in_bridge_space = entity_rotation_matrix.inverse() * bridge_to_player;
      float midpoint_ratio = 1.f - abs(player_position_in_bridge_space.x) / entity.scale.x;
      float floor_height = Tachyon_EaseOutQuad(midpoint_ratio);
      float player_y = floor_height * entity.scale.y / 2.2f;

      AllowPlayerMovement(state, player_y, bridge_plane);

      state.is_on_solid_platform = true;

      break;
    }
  }
}

static void HandleWoodenBridgeCollisions(Tachyon* tachyon, State& state) {
  for (auto& entity : state.wooden_bridges) {
    if (entity.visible_scale == tVec3f(0.f)) continue;

    auto bridge_plane = CollisionSystem::CreatePlane(entity.visible_position, entity.visible_scale * tVec3f(1.1f, 0, 0.4f), entity.visible_rotation);

    if (CollisionSystem::IsPointOnPlane(state.player_position.xz(), bridge_plane)) {
      float player_y = entity.visible_position.y + PLAYER_HEIGHT;

      AllowPlayerMovement(state, player_y, bridge_plane);

      state.is_on_solid_platform = true;

      break;
    }
  }
}

static void HandleAltarCollisions(Tachyon* tachyon, State& state) {
  tVec3f player_xz = state.player_position.xz();

  for (auto& entity : state.altars) {
    tVec3f collision_scale = entity.scale * tVec3f(1.9f, 1.f, 0.45f);

    // Prevent walking into the center statue
    tVec3f statue_center_position = UnitEntityToWorldPosition(entity, tVec3f(-0.5f, 0, 0));

    ResolveSingleRadiusCollision(state, statue_center_position, entity.scale * 0.7f, 1.f);

    // Altar ramp + platform
    auto altar_plane = CollisionSystem::CreatePlane(entity.position, collision_scale, entity.orientation);
    float altar_floor_y = (entity.position.y + PLAYER_HEIGHT) + entity.scale.y * 0.35f;

    if (CollisionSystem::IsPointOnPlane(player_xz, altar_plane)) {
      tVec3f entity_to_player = state.player_position - entity.position;
      tVec3f player_position_in_entity_space = entity.orientation.toMatrix4f().inverse() * entity_to_player;
      float progress_along_x = player_position_in_entity_space.x / collision_scale.x;
      float player_y;

      if (progress_along_x > 0.f) {
        // Walking up the ramp onto the altar
        float ramp_alpha = 1.f - progress_along_x;
        ramp_alpha *= 2.f; // Advance faster to match the altar ramp geometry
        if (ramp_alpha > 1.f) ramp_alpha = 1.f;

        player_y = Tachyon_Lerpf(0.f, altar_floor_y, ramp_alpha);
      } else {
        // On the altar platform
        player_y = altar_floor_y;
      }

      AllowPlayerMovement(state, player_y, altar_plane);

      state.is_on_solid_platform = true;

    // Standing on top of altar platform disc
    } else if (
      tVec3f::distance(player_xz, entity.position.xz()) < (entity.scale.x + PLAYER_RADIUS) &&
      state.player_position.y >= altar_floor_y
    ) {
      AllowPlayerMovement(state, altar_floor_y, altar_plane);

      state.is_on_solid_platform = true;

    // Prevent movement into the altar platform disc from ground level
    } else if (state.fall_velocity == 0.f) {
      ResolveSingleRadiusCollision(state, entity.position, entity.scale, 1.f);
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

    auto log_plane = CollisionSystem::CreatePlane(log.position, log.scale * tVec3f(0.2f, 1.f, 1.f), log.rotation);

    if (CollisionSystem::IsPointOnPlane(state.player_position.xz(), log_plane)) {
      float player_y = log.position.y + log.scale.y * 0.2f + PLAYER_HEIGHT;

      AllowPlayerMovement(state, player_y, log_plane);

      break;
    }
  }
}

static void HandleWaterWheelCollisions(Tachyon* tachyon, State& state) {
  tVec3f player_xz = state.player_position.xz();

  for (auto& entity : state.water_wheels) {
    bool is_turning = (
      state.astro_time <= entity.astro_end_time &&
      !entity.did_activate
    );

    // Wheel collision
    if (!is_turning) {
      // @todo use wheel platforms
      auto wheel_plane = CollisionSystem::CreatePlane(entity.position, entity.scale * tVec3f(1.5f, 1.f, 0.5f), entity.orientation);

      if (CollisionSystem::IsPointOnPlane(player_xz, wheel_plane)) {
        tVec3f entity_to_player = state.player_position - entity.position;
        tVec3f player_position_in_entity_space = entity.orientation.toMatrix4f().inverse() * entity_to_player;

        float progress_along_z = abs(player_position_in_entity_space.z) / (entity.scale.z * 0.6f);
        if (progress_along_z > 1.f) progress_along_z = 1.f;

        float height_alpha = powf(1.f - progress_along_z, 0.6f);
        if (height_alpha < 0.f) height_alpha = 0.f;
        if (height_alpha > 1.f) height_alpha = 1.f;

        // @todo base height should be based on the last flat ground/surface
        float base_y = 0.f;
        float target_y = entity.position.y + entity.scale.y * 0.1f + PLAYER_HEIGHT;
        float player_y = target_y;// Tachyon_Lerpf(base_y, target_y, height_alpha);

        // @temporary
        if (abs(state.last_player_position.y) < 0.01f) {
          PlayerCharacter::AutoHop(tachyon, state);
        }

        AllowPlayerMovement(state, player_y, wheel_plane);

        state.is_on_solid_platform = true;

        continue;
      }
    }

    // Platform collision
    tVec3f platform_center_position = UnitEntityToWorldPosition(entity, tVec3f(0, 0, 0.75f));
    auto platform_plane = CollisionSystem::CreatePlane(platform_center_position, entity.scale * tVec3f(1.5f, 1.f, 0.25f), entity.orientation);

    if (CollisionSystem::IsPointOnPlane(player_xz, platform_plane)) {
      float player_y = 0.f;// entity.position.y - entity.scale.y * 0.1f + PLAYER_HEIGHT;

      // @temporary
      if (state.current_ground_y > player_y) {
        PlayerCharacter::AutoHop(tachyon, state);
      }

      AllowPlayerMovement(state, player_y, platform_plane);

      state.is_on_solid_platform = true;
    }
  }
}

static void HandleMovementOffSolidGround(State& state) {
  tVec3f edge = GetOversteppedEdge(state.player_position, state.last_plane_walked_on);
  float edge_dot = tVec3f::dot(state.player_velocity, edge);
  tVec3f corrected_direction = edge_dot > 0.f ? edge.unit() : edge.invert().unit();
  tVec3f rebound_direction = tVec3f::cross(tVec3f(0, 1.f, 0), edge.unit());

  state.player_position = state.last_solid_ground_position + rebound_direction * 1.f;
  state.player_velocity = corrected_direction * state.player_velocity.magnitude();

  state.current_ground_y = state.last_solid_ground_position.y;
}

bool CollisionSystem::IsPointOnPlane(const tVec3f& point, const Plane& plane) {
  bool d1 = IsPointInsideEdge(point, plane.p2, plane.p1);
  bool d2 = IsPointInsideEdge(point, plane.p3, plane.p2);
  bool d3 = IsPointInsideEdge(point, plane.p4, plane.p3);
  bool d4 = IsPointInsideEdge(point, plane.p1, plane.p4);

  return d1 && d2 && d3 && d4;
}

Plane CollisionSystem::CreatePlane(const tVec3f& position, const tVec3f& scale, const Quaternion& rotation) {
  Plane plane;
  plane.p1 = tVec3f(-1.f, 0, 1.f) * scale;
  plane.p2 = tVec3f(1.f, 0, 1.f) * scale;
  plane.p3 = tVec3f(1.f, 0, -1.f) * scale;
  plane.p4 = tVec3f(-1.f, 0, -1.f) * scale;

  tMat4f rotation_matrix = rotation.toMatrix4f();

  plane.p1 = position + rotation_matrix * plane.p1;
  plane.p2 = position + rotation_matrix * plane.p2;
  plane.p3 = position + rotation_matrix * plane.p3;
  plane.p4 = position + rotation_matrix * plane.p4;

  return plane;
}

void CollisionSystem::HandleCollisions(Tachyon* tachyon, State& state) {
  profile("HandleCollisions()");

  // Assume we're not on solid ground/platforms until
  // such collisions determines otherwise
  state.is_on_solid_ground = false;
  state.is_on_solid_platform = false;

  // If we later resolve a radius collision, this is useful
  // to know in the context of resolving plane collisions
  state.did_resolve_radius_collision = false;

  // @todo prevent out-of-bounds stuff
  for (auto& entity : state.shrubs) {
    if (entity.visible_scale.y < 500.f) continue;

    float radius = entity.visible_scale.x * 1.5f;

    ResolveSoftRadiusCollision(state, entity.position, radius);
  }

  for (auto& entity : state.lilac_bushes) {
    if (entity.visible_scale.x < 500.f) continue;

    float radius = entity.visible_scale.x * 1.2f;

    ResolveSoftRadiusCollision(state, entity.position, radius);
  }

  for (auto& entity : state.oak_trees) {
    ResolveSingleRadiusCollision(state, entity.position, entity.visible_scale, 0.5f);
  }

  for (auto& entity : state.willow_trees) {
    ResolveSingleRadiusCollision(state, entity.position, entity.visible_scale, 0.5f);
  }

  for (auto& entity : state.chestnut_trees) {
    ResolveSingleRadiusCollision(state, entity.position, entity.visible_scale, 0.6f);
  }

  // @temporary (?)
  float player_bottom_y = state.player_position.y - PLAYER_HEIGHT;

  for (auto& rock : objects(state.meshes.rock_1)) {
    if (rock.position.y + rock.scale.y < player_bottom_y) continue;

    ResolveSingleRadiusCollision(state, rock.position, rock.scale, 1.f);
  }

  for (auto& rock : objects(state.meshes.rock_2)) {
    if (rock.position.y + rock.scale.y < player_bottom_y) continue;

    ResolveSingleRadiusCollision(state, rock.position, rock.scale, 1.f);
  }

  for (auto& entity : state.light_posts) {
    ResolveSingleRadiusCollision(state, entity.position, entity.scale, 0.6f);
  }

  for (auto& entity : state.lampposts) {
    ResolveSingleRadiusCollision(state, entity.position, entity.visible_scale, 0.5f);
  }

  for (auto& entity : state.npcs) {
    ResolveSingleRadiusCollision(state, entity.visible_position, entity.scale, 1.5f);
  }

  for (auto& ground : objects(state.meshes.ground_1)) {
    bool has_collision = true;

    // Make an exception for ground_1 objects near small stone bridges.
    //
    // @optimize Don't do this in an inner loop here!
    // Look up ground_1 objects close to bridge entities
    // ahead of time, store them in a collision list, and
    // loop over those collision bounds instead of all
    // of the objects in the outer loop. We probably need
    // to loop over collision bounds in the first place,
    // rather than just collidable objects.
    for (auto& entity : state.small_stone_bridges) {
      float distance_threshold = entity.visible_scale.x * 1.5f;

      if (tVec3f::distance(ground.position, entity.visible_position) < distance_threshold) {
        has_collision = false;

        break;
      }
    }

    if (!has_collision) continue;

    ResolveSingleRadiusCollision(state, ground.position, ground.scale, 0.8f);
  }

  state.current_ground_y = state.water_level + PLAYER_HEIGHT;

  // Resolve collisions with bounded-off rectangular entities
  HandleGateCollisions(tachyon, state);
  HandleWoodenFenceCollisions(tachyon, state);
  HandleHouseCollisions(tachyon, state);

  // Resolve collisions with irregularly-shaped entities
  HandleSmallStoneBridgeCollisions(tachyon, state);
  HandleWoodenBridgeCollisions(tachyon, state);
  HandleAltarCollisions(tachyon, state);
  // HandleRiverLogCollisions(tachyon, state); // @todo remove (?)
  HandleWaterWheelCollisions(tachyon, state);

  // Handle walking on flat ground
  if (!state.is_on_solid_platform) {
    HandleFlatGroundCollisions(tachyon, state);
  }

  // @todo don't do this during astro travel, so that we can
  // fall off platforms etc. if they disappear
  if (!state.is_on_solid_ground) {
    HandleMovementOffSolidGround(state);
  }

  // Falling behavior
  // @todo factor
  {
    if (time_since(state.last_auto_hop_time) > 0.3f) {
      if (state.player_position.y - state.current_ground_y > 100.f) {
        state.fall_velocity += 50000.f * state.dt;
      }

      state.player_position.y -= state.fall_velocity * state.dt;

      if (
        state.current_ground_y - state.player_position.y > 200.f &&
        state.current_ground_y - state.player_position.y < 900.f
      ) {
        PlayerCharacter::AutoHop(tachyon, state);
      }
      else if (
        state.player_position.y < state.current_ground_y ||
        (
          state.player_position.y > state.current_ground_y &&
          state.player_position.y - state.current_ground_y < 100.f
        )
      ) {
        state.player_position.y = state.current_ground_y;
        state.fall_velocity = 0.f;
      }
    }
  }
}