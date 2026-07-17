#include <algorithm>

#include "astro/collision_system.h"
#include "astro/entity_behaviors/behavior.h"
#include "astro/player_character.h"
#include "astro/player_animation.h"
#include "astro/sound_driver.h"

// @temporary
#include "astro/animation.h"

using namespace astro;

// @todo move to constants
static std::vector<float> small_hop_bounce_curve = {
  0.f,
  -0.1f,
  0.6f,
  1.2f,
  1.4f,
  1.5f,
  1.4f,
  1.f,
  1.f
};

// @todo move elsewhere
// @todo allow spline sampling
static float SampleCurveForward(const std::vector<float>& curve, const float t) {
  int max = (int) curve.size();
  float max_time = (float) curve.size();
  float seek_time = t * float(max);
  if (seek_time < 0.f) seek_time += max_time;

  int start_frame = (int) seek_time;
  int end_frame = start_frame + 1;

  if (end_frame >= max - 1) end_frame = max - 1;

  auto a = curve[start_frame % max];
  auto b = curve[end_frame % max];
  float alpha = fmodf(seek_time, 1.f);

  return Tachyon_Lerpf(a, b, alpha);
}

// @todo move to PlayerCharacter.h
constexpr static float PLAYER_RADIUS = 600.f;
constexpr static float PLAYER_HEIGHT = 1500.f;

// @todo improve this to show a collision volume
static void ShowDebugPlane(Tachyon* tachyon, State& state, const Plane& plane) {
  auto& meshes = state.meshes;

  auto& p1 = use_instance(meshes.debug_collision_point);
  auto& p2 = use_instance(meshes.debug_collision_point);
  auto& p3 = use_instance(meshes.debug_collision_point);
  auto& p4 = use_instance(meshes.debug_collision_point);

  p1.position = plane.p1;
  p2.position = plane.p2;
  p3.position = plane.p3;
  p4.position = plane.p4;

  p1.scale = tVec3f(50.f, 3000.f, 50.f);
  p2.scale = tVec3f(50.f, 3000.f, 50.f);
  p3.scale = tVec3f(50.f, 3000.f, 50.f);
  p4.scale = tVec3f(50.f, 3000.f, 50.f);

  p1.color = tVec3f(0, 0, 1.f);
  p2.color = tVec3f(0, 0, 1.f);
  p3.color = tVec3f(0, 0, 1.f);
  p4.color = tVec3f(0, 0, 1.f);

  commit(p1);
  commit(p2);
  commit(p3);
  commit(p4);
}

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

      // If we haven't resolved an earlier plane collision, which
      // represents a new "source of truth" for position resolution,
      // push the player out/slide along the edge of this plane
      if (!state.did_resolve_plane_collision) {
        // Add rebound to mitigate any accidental clipping in
        state.player_position += rebound_direction * 0.1f;
        // Slide along the egde to conserve movement
        state.player_position += corrected_direction * corrected_speed;
      }
    }

    state.did_resolve_plane_collision = true;

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
    if (!IsDuringActiveTime(entity, state)) continue;

    auto fence_plane = CollisionSystem::CreatePlane(entity.position, entity.visible_scale, entity.orientation);

    ResolveClippingIntoPlane(state, fence_plane);
  }
}

static void HandleHouseCollisions(Tachyon* tachyon, State& state) {
  for (auto& entity : state.houses) {
    if (entity.visible_scale.x == 0.f) continue;

    auto house_plane = CollisionSystem::CreatePlane(entity.position, entity.visible_scale * 1.2f, entity.orientation);

    ResolveClippingIntoPlane(state, house_plane);
  }
}

static void HandleCastleStairsCollisions(Tachyon* tachyon, State& state) {
  tVec3f player_xz = state.player_position.xz();

  for (auto& entity : state.castle_stairs) {
    if (!IsDuringActiveTime(entity, state)) continue;

    auto slope_plane = CollisionSystem::CreatePlane(entity.position, entity.scale, entity.orientation);

    if (CollisionSystem::IsPointOnPlane(player_xz, slope_plane)) {
      // Apply the right footstep sounds
      state.is_on_stone_surface = true;

      // Only use slope collision if the stairs are static (not trigger-activated),
      // or already activated
      if (!entity.requires_action || entity.did_activate) {
        // Figure out how far along the slope the player is,
        // and set their height accordingly
        //
        // @todo the slope runs along the object-space x axis,
        // which is counterintuitive. It should be z, ideally.
        tVec3f slope_to_player = state.player_position - entity.position;
        tVec3f player_position_in_slope_space = entity.orientation.toMatrix4f().inverse() * slope_to_player;
        float progress_along_slope = 0.5f * (1.f - player_position_in_slope_space.x / entity.scale.x);
        float slope_bottom_y = slope_plane.p1.y + PLAYER_HEIGHT + 200.f;
        float player_y = slope_bottom_y + progress_along_slope * entity.scale.y * 1.15f;
        float slope_dot = tVec3f::dot(state.player_facing_direction, entity.orientation.getLeftDirection());

        AllowPlayerMovement(state, player_y, slope_plane);

        state.is_on_solid_platform = true;
        state.is_moving_down_slope = slope_dot < -0.4f || slope_dot > 0.4f;
        state.player.ledge_jump_duration = 0.2f;
      }
    }
  }
}

static void HandleSlopeCollisions(Tachyon* tachyon, State& state) {
  tVec3f player_xz = state.player_position.xz();

  for (auto& slope : objects(state.meshes.stairs_floor)) {
    auto slope_plane = CollisionSystem::CreatePlane(slope.position, slope.scale, slope.rotation);

    if (CollisionSystem::IsPointOnPlane(player_xz, slope_plane)) {
      // Figure out how far along the slope the player is,
      // and set their height accordingly
      //
      // @todo the slope runs along the object-space x axis,
      // which is counterintuitive. It should be z, ideally.
      tVec3f slope_to_player = state.player_position - slope.position;
      tVec3f player_position_in_slope_space = slope.rotation.toMatrix4f().inverse() * slope_to_player;
      float progress_along_slope = 0.5f * (1.f - player_position_in_slope_space.x / slope.scale.x);
      float slope_bottom_y = slope_plane.p1.y + PLAYER_HEIGHT;
      float desired_player_y = slope_bottom_y + progress_along_slope * slope.scale.y;
      float slope_dot = tVec3f::dot(state.player_facing_direction, slope.rotation.getLeftDirection());

      // Don't snap to y unless we're close enough to the slope surface
      if (state.player_position.y > desired_player_y + 200.f) {
        continue;
      }

      AllowPlayerMovement(state, desired_player_y, slope_plane);

      state.is_on_solid_platform = true;
      state.is_on_stone_surface = true;
      state.is_moving_down_slope = slope_dot < -0.4f || slope_dot > 0.4f;
      state.player.ledge_jump_duration = 0.2f;
    }
  }
}

static void HandleNormalSwitchCollisions(Tachyon* tachyon, State& state) {
  for (auto& entity : state.normal_switches) {
    if (!IsDuringActiveTime(entity, state)) continue;

    float dx = abs(state.player_position.x - entity.position.x);
    float dz = abs(state.player_position.z - entity.position.z);

    if (dx < entity.scale.x && dz < entity.scale.z) {
      auto plane = CollisionSystem::CreatePlane(entity.position, entity.scale, entity.orientation);
      float player_y = entity.position.y + PLAYER_HEIGHT + 50.f;

      AllowPlayerMovement(state, player_y, plane);

      state.is_on_solid_platform = true;
      state.is_on_wood_surface = true;

      return;
    }
  }
}

// @todo move this to top
static void HandleLadderCollisions(Tachyon* tachyon, State& state) {
  float scene_time = get_scene_time();

  if (PlayerCharacter::IsClimbingOffLadder(tachyon, state)) {
    return;
  }

  bool is_left_stick_up = tachyon->left_stick.y < 0.f;
  bool is_left_stick_down = tachyon->left_stick.y > 0.f;
  bool was_just_climbing = time_since(state.player.last_climbing_time) < 0.5f;

  bool is_jumping_off_wall_ladder = (
    state.player.rig.current_animation == &state.animations.player_climb_up_jump ||
    state.player.rig.current_animation == &state.animations.player_freefall2
  );

  for (auto& entity : state.ladders) {
    if (!IsDuringActiveTime(entity, state)) continue;

    float dx = abs(state.player_position.x - entity.position.x);
    float dz = abs(state.player_position.z - entity.position.z);

    tVec3f player_to_entity = (entity.position - state.player_position).xz().unit();
    float moving_dot = tVec3f::dot(state.player_velocity.xz(), player_to_entity);
    bool is_moving_toward_ladder = moving_dot > 0.f;
    float ladder_top_y = entity.position.y + entity.scale.y;
    float climbing_top_y = ladder_top_y - 1500.f;
    float climbing_bottom_y = entity.position.y - entity.scale.y + 2500.f;

    // Approaching the bottom or top of a ladder
    bool is_climbing_onto_ladder_normally = (
      dx < 1200.f && dz < 1200.f &&
      !state.did_climb_up_jump &&
      !state.did_jump_off_ledge &&
      (is_moving_toward_ladder || was_just_climbing)
    );

    // Approaching a ladder over the edge of a raised wall
    bool is_climbing_over_wall_onto_ladder = (
      !state.is_on_ladder &&
      !is_jumping_off_wall_ladder &&
      state.player_position.y > entity.position.y &&
      entity.requires_action &&
      dx < 1600.f && dz < 1600.f
    );

    if (is_climbing_onto_ladder_normally || is_climbing_over_wall_onto_ladder) {
      state.player.last_climbing_time = scene_time;

      if (!was_just_climbing) {
        // Starting a new climb action, so track its time
        state.player.last_climbing_start_time = scene_time;

        if (is_climbing_over_wall_onto_ladder) {
          state.player.is_hopping_up_to_climb_down = true;

          // @temporary
          state.is_on_stone_surface = true;

          SoundDriver::PlayWalkSound(state, 0.6f);
        } else {
          SoundDriver::PlayLadderSound(state, 1.f);
        }
      }

      float time_since_starting_climb = time_since(state.player.last_climbing_start_time);

      bool did_climb_off_top = (
        is_left_stick_up &&
        !state.player.is_hopping_up_to_climb_down &&
        !state.player.is_turning_to_climb_down &&
        !state.player.is_starting_climb_down &&
        time_since_starting_climb > 0.5f &&
        state.player_position.y > climbing_top_y
      );

      bool did_climb_off_bottom = (
        is_left_stick_down &&
        time_since_starting_climb > 0.5f &&
        state.player_position.y < climbing_bottom_y
      );

      tVec3f climbing_position_xz = UnitEntityToWorldPosition(entity, tVec3f(1.1f, 0, 0)).xz();

      if (did_climb_off_top) {
        state.player_velocity = tVec3f(0.f);
        state.player.last_climbing_stop_time = scene_time;
        state.did_climb_down = false;
        state.player.climb_up_start_position = state.player_position;
        state.is_on_ladder = false;
        state.did_climb_up_jump = entity.requires_action;

        if (state.did_climb_up_jump) {
          state.player.ledge_jump_duration = 0.f;
        }
      }
      else if (did_climb_off_bottom) {
        tVec3f off_direction = (climbing_position_xz - entity.position.xz()).unit();

        state.player_position.y = climbing_bottom_y;
        state.player_velocity = off_direction * 1000.f;
        state.player.last_climbing_stop_time = scene_time;
        state.did_climb_down = true;
        state.did_climb_up_jump = false;
        state.is_on_ladder = false;

        // Tentatively assume the ground is some distance below us,
        // and reset our fall velocity. As soon as we start falling
        // the actual ground y will be updated.
        state.current_ground_y = state.player_position.y - 1000.f;
        state.fall_velocity = 0.f;
      }
      else if (state.player.is_hopping_up_to_climb_down) {
        float time_since_starting_climb = time_since(state.player.last_climbing_start_time);

        if (time_since_starting_climb < 0.7f) {
          // Hop up
          float hop_alpha = time_since_starting_climb / 0.7f;

          // Blend into the climbing position
          tVec3f direction = (climbing_position_xz - state.player_position.xz()).unit();

          state.player_velocity = tVec3f(0.f);
          state.player_position += direction * 1500.f * state.dt;

          // @temporary
          // @todo store player y at start of hop
          float base_y = ladder_top_y - 300.f;
          float hop_height = (ladder_top_y + 1000.f) - base_y;

          float sample = SampleCurveForward(small_hop_bounce_curve, hop_alpha);

          state.player_position.y = base_y + hop_height * sample;
        } else {
          // Once we finish the hop up, we're going to start turning
          // so we can climb down onto the ladder. Pre-emptively set
          // the flag so we can't control the player until they're on
          // the ladder completely.
          state.player.is_hopping_up_to_climb_down = false;
          state.player.is_turning_to_climb_down = true;
        }

        state.is_on_ladder = true;
        state.did_jump_off_ledge = false;
      }
      else {
        float alpha = 3.f * state.dt;

        // Blend into the climbing position
        state.player_position.x = Tachyon_Lerpf(state.player_position.x, climbing_position_xz.x, alpha);
        state.player_position.z = Tachyon_Lerpf(state.player_position.z, climbing_position_xz.z, alpha);

        // Blend into the ladder-facing direction
        tVec3f desired_facing_direction = (entity.position.xz() - climbing_position_xz).unit();

        state.player_facing_direction = tVec3f::slerp(
          state.player_facing_direction,
          desired_facing_direction,
          6.f * state.dt
        );

        if (state.player_position.y > climbing_top_y) {
          float facing_dot = tVec3f::dot(state.player_facing_direction, desired_facing_direction);

          state.player.climb_down_onto_something_remaining_y = state.player_position.y - climbing_top_y;

          if (facing_dot > 0.8f) {
            // Climbing down until we're below the climbing y limit
            state.player.is_starting_climb_down = true;
            state.player.is_turning_to_climb_down = false;
            state.player_position.y -= 2500.f * state.dt;
          } else {
            // Turning around so we can start climbing down
            state.player.is_turning_to_climb_down = true;
          }
        } else {
          state.player.is_starting_climb_down = false;
        }

        state.is_on_ladder = true;
        state.did_jump_off_ledge = false;

        break;
      }
    }
  }
}

static void HandleFlatGroundCollisions(Tachyon* tachyon, State& state) {
  profile("HandleFlatGroundCollisions()");

  for (auto& plane : state.flat_ground_planes) {
    if (IsPointWithRadiusOnPlane(state.player_position, 400.f, plane)) {
      float ground_y = plane.p1.y + PLAYER_HEIGHT;

      AllowPlayerMovement(state, ground_y, plane);

      break;
    }
  }
}

static void HandleSmallStoneBridgeCollisions(Tachyon* tachyon, State& state) {
  for (auto& entity : state.small_stone_bridges) {
    if (entity.visible_scale == tVec3f(0.f)) continue;

    tMat4f entity_rotation_matrix = entity.orientation.toMatrix4f();

    // Edge/wall collision
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
      float base_height = entity.position.y + PLAYER_HEIGHT;
      float floor_height = Tachyon_EaseOutQuad(midpoint_ratio);
      float player_y = base_height + floor_height * entity.scale.y * 0.36f;

      AllowPlayerMovement(state, player_y, bridge_plane);

      state.is_on_solid_platform = true;
      state.is_on_stone_surface = true;
      state.player.ledge_jump_duration = 0.2f;

      break;
    }
  }
}

static void HandleWoodenBridgeCollisions(Tachyon* tachyon, State& state) {
  for (auto& entity : state.wooden_bridges) {
    // Ignore bridges not currently active (@todo use astro start/end times)
    if (entity.visible_scale == tVec3f(0.f)) continue;

    // Ignore bridges not level with the player's y position
    if (abs(state.player_position.y - entity.visible_position.y) > 2000.f) continue;

    auto bridge_plane = CollisionSystem::CreatePlane(entity.visible_position, entity.visible_scale * tVec3f(1.1f, 0, 0.4f), entity.visible_rotation);

    if (CollisionSystem::IsPointOnPlane(state.player_position.xz(), bridge_plane)) {
      float player_y = entity.visible_position.y + PLAYER_HEIGHT;

      AllowPlayerMovement(state, player_y, bridge_plane);

      state.is_on_solid_platform = true;
      state.is_on_wood_surface = true;
      state.player.ledge_jump_duration = 0.2f;

      break;
    }
  }
}

static void HandleIronGateCollisions(Tachyon* tachyon, State& state) {
  for (auto& entity : state.iron_gates) {
    if (entity.did_activate) {
      // Opened gate collision
      tVec3f left_wall_position = UnitEntityToWorldPosition(entity, tVec3f(0.5f, 0, 0));
      tVec3f right_wall_position = UnitEntityToWorldPosition(entity, tVec3f(-0.5f, 0, 0));

      auto left_wall_plane = CollisionSystem::CreatePlane(
        left_wall_position,
        entity.scale * tVec3f(0.2f),
        entity.orientation
      );

      auto right_wall_plane = CollisionSystem::CreatePlane(
        right_wall_position,
        entity.scale * tVec3f(0.2f),
        entity.orientation
      );

      ResolveClippingIntoPlane(state, left_wall_plane);
      ResolveClippingIntoPlane(state, right_wall_plane);
    } else {
      // Closed gate collision
      auto gate_plane = CollisionSystem::CreatePlane(
        entity.position,
        entity.scale * tVec3f(0.75f, 1.f, 0.35f),
        entity.orientation
      );

      ResolveClippingIntoPlane(state, gate_plane);
    }
  }
}

static void HandleBirdGateCollisions(Tachyon* tachyon, State& state) {
  for (auto& entity : state.bird_gates) {
    bool is_open = entity.did_activate;

    if (is_open) {
      tVec3f left_door_position = UnitEntityToWorldPosition(entity, tVec3f(0.7f, 0.f, -0.35f));
      tVec3f right_door_position = UnitEntityToWorldPosition(entity, tVec3f(0.7f, 0.f, 0.35f));

      Quaternion left_door_rotation = entity.orientation * Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), 1.2f);
      Quaternion right_door_rotation = entity.orientation * Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), -1.2f);

      auto left_door_plane = CollisionSystem::CreatePlane(
        left_door_position,
        entity.scale * tVec3f(0.2f, 1.f, 0.5f),
        left_door_rotation
      );

      auto right_door_plane = CollisionSystem::CreatePlane(
        right_door_position,
        entity.scale * tVec3f(0.2f, 1.f, 0.5f),
        right_door_rotation
      );

      ResolveClippingIntoPlane(state, left_door_plane);
      ResolveClippingIntoPlane(state, right_door_plane);
    } else {
      auto gate_plane = CollisionSystem::CreatePlane(
        entity.position,
        entity.scale * tVec3f(0.2f, 1.f, 1.f),
        entity.orientation
      );

      ResolveClippingIntoPlane(state, gate_plane);
    }
  }
}

static void HandleCastleRampartCollisions(Tachyon* tachyon, State& state) {
  tVec3f player_xz = state.player_position.xz();
  float player_bottom_y = state.player_position.y - PLAYER_HEIGHT;

  for (auto& entity : state.castle_ramparts) {
    auto rampart_plane = CollisionSystem::CreatePlane(
      entity.position,
      entity.scale * tVec3f(0.2f, 1.f, 0.75f),
      entity.orientation
    );

    if (player_bottom_y > entity.position.y) {
      // Act as a floor
      float floor_y = entity.position.y + entity.scale.y * 0.25f;

      if (CollisionSystem::IsPointOnPlane(player_xz, rampart_plane)) {
        AllowPlayerMovement(state, floor_y + PLAYER_HEIGHT, rampart_plane);

        state.is_on_solid_platform = true;
        state.is_on_stone_surface = true;

        // Do small jumps off ramparts
        state.player.ledge_jump_duration = 0.1f;
      }
    } else {
      // Act as a wall
      ResolveClippingIntoPlane(state, rampart_plane);
    }
  }
}

static void HandleCastleTowerCollisions(Tachyon* tachyon, State& state) {
  float player_body_y = state.player_position.y + PLAYER_HEIGHT;
  float highest_y = -FLT_MAX;

  for (auto& entity : state.castle_towers) {
    float tower_top_y = entity.position.y + entity.scale.y;

    // if (tower_top_y > player_body_y) {
    if (entity.position.y > player_body_y) { // @temporary
      // Act as a wall

      // @todo
    } else if (tower_top_y > highest_y) {
      // Allow movement on top of the tower
      auto tower_plane = CollisionSystem::CreatePlane(entity.position, entity.scale * tVec3f(0.6f, 1.f, 0.6f), entity.orientation);
      float player_top_y = tower_top_y + PLAYER_HEIGHT;

      if (
        state.player_position.y <= player_top_y &&
        CollisionSystem::IsPointOnPlane(state.player_position, tower_plane)
      ) {
        highest_y = tower_top_y;

        AllowPlayerMovement(state, player_top_y, tower_plane);

        state.is_on_solid_platform = true;
        state.is_on_stone_surface = true;

        // Extend ledge jumps off castle walls
        state.player.ledge_jump_duration = 0.32f;
      }
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
      state.player.ledge_jump_duration = 0.2f;

    // Standing on top of altar platform disc
    } else if (
      tVec3f::distance(player_xz, entity.position.xz()) < (entity.scale.x + PLAYER_RADIUS) &&
      state.player_position.y >= altar_floor_y
    ) {
      AllowPlayerMovement(state, altar_floor_y, altar_plane);

      state.is_on_solid_platform = true;
      state.player.ledge_jump_duration = 0.2f;

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

        AllowPlayerMovement(state, player_y, wheel_plane);

        state.is_on_solid_platform = true;
        state.player.ledge_jump_duration = 0.2f;

        continue;
      }
    }

    // Platform collision
    tVec3f platform_center_position = UnitEntityToWorldPosition(entity, tVec3f(0, 0, 0.75f));
    auto platform_plane = CollisionSystem::CreatePlane(platform_center_position, entity.scale * tVec3f(1.5f, 1.f, 0.25f), entity.orientation);

    if (CollisionSystem::IsPointOnPlane(player_xz, platform_plane)) {
      float player_y = 0.f;// entity.position.y - entity.scale.y * 0.1f + PLAYER_HEIGHT;

      AllowPlayerMovement(state, player_y, platform_plane);

      state.is_on_solid_platform = true;
      state.player.ledge_jump_duration = 0.2f;
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

bool CollisionSystem::IsPointOnPlane(const tVec3f& point, const Plane& plane) {
  bool d1 = IsPointInsideEdge(point, plane.p2, plane.p1);
  bool d2 = IsPointInsideEdge(point, plane.p3, plane.p2);
  bool d3 = IsPointInsideEdge(point, plane.p4, plane.p3);
  bool d4 = IsPointInsideEdge(point, plane.p1, plane.p4);

  return d1 && d2 && d3 && d4;
}

void CollisionSystem::RebuildFlatGroundPlanes(Tachyon* tachyon, State& state) {
  auto& planes = state.flat_ground_planes;

  planes.clear();

  for (auto& flat_ground : objects(state.meshes.flat_ground)) {
    auto plane = CollisionSystem::CreatePlane(flat_ground.position, flat_ground.scale, flat_ground.rotation);

    planes.push_back(plane);
  }

  std::sort(planes.begin(), planes.end(), [&](Plane& a, Plane& b) {
    return a.p1.y > b.p1.y;
  });
}

float CollisionSystem::QueryGroundHeight(State& state, const float x, const float z) {
  tVec3f point = tVec3f(x, 0.f, z);

  for (auto& plane : state.flat_ground_planes) {
    if (CollisionSystem::IsPointOnPlane(point, plane)) {
      return plane.p1.y;
    }
  }

  return -3000.f;
}

void CollisionSystem::HandleCollisions(Tachyon* tachyon, State& state) {
  profile("HandleCollisions()");

  // @todo dev mode only
  {
    reset_instances(state.meshes.debug_collision_point);
  }

  // Assume these conditions are false unless otherwise determined
  state.is_on_solid_ground = false;
  state.is_on_solid_platform = false;
  state.is_moving_down_slope = false;
  state.is_on_wood_surface = false;
  state.is_on_stone_surface = false;
  state.is_on_ladder = false;

  // If we later resolve radius/plane collisions, this is useful
  // to know in the context of resolving other kinds of collisions
  state.did_resolve_radius_collision = false;
  state.did_resolve_plane_collision = false;

  if (
    PlayerCharacter::IsClimbingOffLadder(tachyon, state) &&
    !state.did_climb_down
  ) {
    // @todo PlayerCharacter::ClimbUp()

    auto& animations = state.animations;
    auto& rig = state.player.rig;

    if (
      rig.next_animation == &animations.player_climb_up ||
      rig.next_animation == &animations.player_climb_up_jump
    ) {
      tVec3f root_motion = Animation::GetRootMotion(rig) * 1500.f;
      tVec3f root_offset = state.player.rotation_matrix * root_motion;

      state.player_position = state.player.climb_up_start_position + root_offset;
    }

    state.player_idle_stance = 2;
    state.current_ground_y = state.player_position.y;

    return;
  }

  // Resolve collisions with climbable entities before anything else
  HandleLadderCollisions(tachyon, state);

  // While on ladders, skip normal collision checks
  if (state.is_on_ladder) {
    state.current_ground_y = state.player_position.y;

    return;
  }

  if (PlayerCharacter::IsClimbingOffLadder(tachyon, state) && !state.did_climb_down) {
    return;
  }

  for (auto& entity : state.shrubs) {
    if (entity.visible_scale.y < 500.f) continue;

    float radius = entity.visible_scale.x * 1.5f;

    ResolveSoftRadiusCollision(state, entity.position, radius);
  }

  for (auto& entity : state.leaf_shrubs) {
    // @todo use astro time
    if (entity.visible_scale.x < 500.f) continue;

    // Ignore leaf shrubs well below the player
    if (state.player_position.y - entity.position.y > entity.scale.y) continue;

    float radius = entity.visible_scale.x * 1.2f;

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

  for_used_instances(state.meshes.rock_1) {
    auto& rock = objects(state.meshes.rock_1)[i];

    if (rock.position.y + rock.scale.y < player_bottom_y) continue;

    ResolveSingleRadiusCollision(state, rock.position, rock.scale, 1.f);
  }

  for_used_instances(state.meshes.rock_2) {
    auto& rock = objects(state.meshes.rock_2)[i];

    if (rock.position.y + rock.scale.y < player_bottom_y) continue;

    ResolveSingleRadiusCollision(state, rock.position, rock.scale, 1.f);
  }

  for (auto& entity : state.light_posts) {
    ResolveSingleRadiusCollision(state, entity.position, entity.scale, 0.6f);
  }

  for (auto& entity : state.lampposts) {
    if (!IsDuringActiveTime(entity, state)) continue;

    ResolveSingleRadiusCollision(state, entity.position, entity.visible_scale, 0.5f);
  }

  for (auto& entity : state.wind_chimes) {
    ResolveSingleRadiusCollision(state, entity.position, entity.visible_scale, 1.2f);
  }

  for (auto& entity : state.npcs) {
    if (!IsDuringActiveTime(entity, state)) continue;

    ResolveSingleRadiusCollision(state, entity.visible_position, entity.scale, 1.f);
  }

  for_used_instances(state.meshes.ground_1) {
    auto& ground = objects(state.meshes.ground_1)[i];

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
  HandleIronGateCollisions(tachyon, state);
  HandleBirdGateCollisions(tachyon, state);
  HandleCastleRampartCollisions(tachyon, state);

  // Resolve ground collisions with irregularly-shaped entities
  HandleSmallStoneBridgeCollisions(tachyon, state);
  HandleWoodenBridgeCollisions(tachyon, state);
  HandleCastleTowerCollisions(tachyon, state);
  HandleAltarCollisions(tachyon, state);
  HandleWaterWheelCollisions(tachyon, state);
  HandleCastleStairsCollisions(tachyon, state);
  HandleSlopeCollisions(tachyon, state);

  // Switches
  HandleNormalSwitchCollisions(tachyon, state);

  // HandleRiverLogCollisions(tachyon, state); // @todo remove (?)

  // Handle walking on flat ground
  if (!state.is_on_solid_platform) {
    HandleFlatGroundCollisions(tachyon, state);
  }

  // @todo don't do this during astro travel, so that we can
  // fall off platforms etc. if they disappear
  if (!state.is_on_solid_ground) {
    HandleMovementOffSolidGround(state);
  }

  // Climbing-off behavior
  // @todo factor
  {
    if (PlayerCharacter::IsClimbingOffLadder(tachyon, state)) {
      if (state.did_climb_down) {
        if (state.player_position.y > state.current_ground_y) {
          // Jumping off/falling
          state.fall_velocity += 20000.f * state.dt;
        } else {
          // Hit the ground; blend into the current ground y position
          state.player_position.y = Tachyon_Lerpf(
            state.player_position.y,
            state.current_ground_y,
            2.f * state.dt
          );

          state.fall_velocity = 0.f;
        }
      } else {
        // Climbing up; zero out fall velocity to yield to animation control
        state.fall_velocity = 0.f;
      }

      state.player_position.y -= state.fall_velocity * state.dt;

      return;
    }
  }

  // Landing/falling/grounding behavior
  // @todo refactor/clean up
  {
    float height_above_ground = state.player_position.y - state.current_ground_y;
    float distance_below_ground = -height_above_ground;

    // Start freefall
    if (height_above_ground > 500.f && !state.did_jump_off_ledge) {
      state.did_jump_off_ledge = true;
      state.player.last_ledge_jump_time = get_scene_time();

      if (state.did_climb_up_jump) {
        state.player_velocity = state.player_facing_direction * 1250.f;

        // Apply the velocity update so the player doesn't hang in the air
        // for a split second, since position updates normally occur before
        // all collision checks, in ControlSystem::
        state.player_position += state.player_velocity * 5.f * state.dt;
      }
    }

    // Snap to ground level when only slightly above
    if (
      height_above_ground > 0.f &&
      height_above_ground <= 500.f &&
      !state.did_jump_off_ledge
    ) {
      state.player_position.y = state.current_ground_y;
    }

    // If we're slightly below ground level, blend smoothly to the correct y position
    if (distance_below_ground > 0.f && distance_below_ground < 200.f) {
      state.player_position.y = Tachyon_Lerpf(
        state.player_position.y,
        state.current_ground_y,
        2.f * state.dt
      );
    }

    // Snap our y position if:
    if (
      // We're moving
      state.previous_move_delta > 5.f && (
        // We're on a solid platform (distinct from normal ground)
        state.is_on_solid_platform ||
        // We're far enough below ground level
        distance_below_ground > 200.f ||
        // We're within a small threshold of ground level, snap our y position
        abs(height_above_ground) < 10.f
      )
    ) {
      state.player_position.y = state.current_ground_y;
      state.fall_velocity = 0.f;
    }

    // Falling behavior
    if (
      state.player.last_ledge_jump_time != 0.f &&
      time_since(state.player.last_ledge_jump_time) > state.player.ledge_jump_duration &&
      height_above_ground > 0.f
    ) {
      state.fall_velocity += 50000.f * state.dt;
      state.player_position.y -= state.fall_velocity * state.dt;

      if (state.player_position.y < state.current_ground_y) {
        state.player_position.y = state.current_ground_y;
        state.fall_velocity = 0.f;
      }
    }

    // Stop ledge jumps once on the ground
    if (state.player_position.y == state.current_ground_y) {
      if (state.did_jump_off_ledge) {
        state.player.last_freefall_landing_time = get_scene_time();

        float volume = state.is_on_stone_surface ? 0.5f : 0.25f;

        SoundDriver::PlayWalkSound(state, volume);
      }

      state.did_climb_up_jump = false;
      state.did_jump_off_ledge = false;
      state.player.last_climbing_stop_time = 0.f;

      // Reset ledge jump duration when landed on solid ground,
      // as opposed to a specialized solid platform
      if (!state.is_on_solid_platform) {
        state.player.ledge_jump_duration = 0.2f;
      }
    }
  }
}