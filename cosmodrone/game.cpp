#include "cosmodrone/flight_system.h"
#include "cosmodrone/game.h"
#include "cosmodrone/game_editor.h"
#include "cosmodrone/game_types.h"
#include "cosmodrone/hud_system.h"
#include "cosmodrone/mesh_library.h"
#include "cosmodrone/target_system.h"
#include "cosmodrone/world_behavior.h"
#include "cosmodrone/world_setup.h"

using namespace Cosmodrone;

const static auto FORWARD_VECTOR = tVec3f(0, 0, -1.f);
const static auto UP_VECTOR = tVec3f(0, 1.f, 0);
const static auto RIGHT_VECTOR = tVec3f(1.f, 0, 0);

// @todo pass into StartGame() and RunGame() from main.cpp
static State state;

// @todo move to engine
inline float Lerpf(float a, float b, float alpha) {
  return a + (b - a) * alpha;
}

// @todo move to engine
inline tVec3f Lerpf(const tVec3f& a, const tVec3f& b, const float alpha) {
  return tVec3f(
    Lerpf(a.x, b.x, alpha),
    Lerpf(a.y, b.y, alpha),
    Lerpf(a.z, b.z, alpha)
  );
}

// @todo remove in favor of LookRotation()
static Quaternion DirectionToQuaternion(const tVec3f& direction) {
  auto yaw = atan2f(direction.x, direction.z);
  auto pitch = atan2f(direction.xz().magnitude(), direction.y) - t_HALF_PI;

  return (
    Quaternion::fromAxisAngle(UP_VECTOR, yaw) *
    Quaternion::fromAxisAngle(RIGHT_VECTOR, pitch)
  );
}

/**
 * Adapted from https://forum.playcanvas.com/t/quaternion-from-direction-vector/6369/3
 *
 * @todo move to engine
 */
static Quaternion LookRotation(const tVec3f& forward, const tVec3f& up) {
  auto vector = forward;
  auto vector2 = tVec3f::cross(up, vector).unit();
  auto vector3 = tVec3f::cross(vector, vector2);
  auto m00 = vector2.x;
  auto m01 = vector2.y;
  auto m02 = vector2.z;
  auto m10 = vector3.x;
  auto m11 = vector3.y;
  auto m12 = vector3.z;
  auto m20 = vector.x;
  auto m21 = vector.y;
  auto m22 = vector.z;

  auto num8 = (m00 + m11) + m22;
  Quaternion quaternion;
  if (num8 > 0.f)
  {
      auto num = sqrtf(num8 + 1.f);
      quaternion.w = num * 0.5f;
      num = 0.5f / num;
      quaternion.x = (m12 - m21) * num;
      quaternion.y = (m20 - m02) * num;
      quaternion.z = (m01 - m10) * num;
      return quaternion;
  }
  if ((m00 >= m11) && (m00 >= m22))
  {
      auto num7 = sqrtf(((1.f + m00) - m11) - m22);
      auto num4 = 0.5f / num7;
      quaternion.x = 0.5f * num7;
      quaternion.y = (m01 + m10) * num4;
      quaternion.z = (m02 + m20) * num4;
      quaternion.w = (m12 - m21) * num4;
      return quaternion;
  }
  if (m11 > m22)
  {
      auto num6 = sqrtf(((1.f + m11) - m00) - m22);
      auto num3 = 0.5f / num6;
      quaternion.x = (m10 + m01) * num3;
      quaternion.y = 0.5f * num6;
      quaternion.z = (m21 + m12) * num3;
      quaternion.w = (m20 - m02) * num3;
      return quaternion;
  }
  auto num5 = sqrtf(((1.f + m22) - m00) - m11);
  auto num2 = 0.5f / num5;
  quaternion.x = (m20 + m02) * num2;
  quaternion.y = (m21 + m12) * num2;
  quaternion.z = 0.5f * num5;
  quaternion.w = (m01 - m10) * num2;
  return quaternion;
}

static void UpdateViewDirections(Tachyon* tachyon, State& state) {
  auto& camera = tachyon->scene.camera;

  auto view_matrix = (
    camera.rotation.toMatrix4f() *
    tMat4f::translation(camera.position * tVec3f(-1.f))
  );

  state.view_forward_direction = tVec3f(
    view_matrix.m[8],
    view_matrix.m[9],
    view_matrix.m[10]
  ).invert();

  state.view_up_direction = tVec3f(
    view_matrix.m[4],
    view_matrix.m[5],
    view_matrix.m[6]
  );
}

static tVec3f GetDockingPositionOffset(const State& state) {
  if (state.docking_target.mesh_index == state.meshes.antenna_3) {
    return tVec3f(0, -1.f, -1.f).unit() * 0.7f;
  }

  return tVec3f(0, -1.f, -1.f).unit();
}

static tVec3f GetDockingPosition(Tachyon* tachyon, const State& state) {
  auto* target = get_original_object(state.docking_target);

  if (target == nullptr) {
    return tVec3f(0.f);
  }

  auto& target_rotation = target->rotation;
  auto offset = GetDockingPositionOffset(state);

  offset *= target->scale;
  offset = target_rotation.toMatrix4f() * offset;

  return target->position + offset;
}

static float GetTargetPositionAim(const State& state, const tVec3f& target_position) {
  auto forward = state.ship_rotation_basis.forward;
  auto ship_to_target = (target_position - state.ship_position).unit();

  return tVec3f::dot(forward, ship_to_target);
}

static void AttemptDockingProcedure(State& state) {
  auto* tracker = TargetSystem::GetSelectedTargetTracker(state);

  if (tracker == nullptr) {
    return;
  }

  auto& target_object = tracker->object;
  auto& ship_position = state.ship_position;
  auto target_distance = (target_object.position - ship_position).magnitude();

  if (target_distance > 80000.f) {
    return;
  }

  state.flight_mode = FlightMode::AUTO_DOCK;
  state.auto_dock_stage = AutoDockStage::APPROACH_DECELERATION;
  state.docking_target = target_object;
}

const static float MAX_SHIP_SPEED = 15000.f;

static void HandleFlightControls(Tachyon* tachyon, State& state, const float dt) {
  bool is_issuing_control_action = false;

  // Handle forward thrust
  if (is_key_held(tKey::W)) {
    FlightSystem::ControlledThrustForward(state, dt);

    is_issuing_control_action = true;
  }

  // Enforce maximum ship speed
  float ship_speed = state.ship_velocity.magnitude();

  if (ship_speed > MAX_SHIP_SPEED) {
    state.ship_velocity = state.ship_velocity.unit() * MAX_SHIP_SPEED;
  }

  if (is_key_held(tKey::S)) {
    // Handle pitch up
    FlightSystem::ChangePitch(state, dt, 1.f);

    is_issuing_control_action = true;
  } else {
    // Reduce pitch gradually
    state.ship_pitch_factor *= (1.f - 5.f * dt);

    // Prevent a long reduction tail and just snap to 0
    if (abs(state.ship_pitch_factor) < 0.01f) {
      state.ship_pitch_factor = 0.f;
    }
  }

  // Handle yaw manuevers
  if (is_key_held(tKey::A)) {
    FlightSystem::YawLeft(state, dt);

    is_issuing_control_action = true;
  } else if (is_key_held(tKey::D)) {
    FlightSystem::YawRight(state, dt);

    is_issuing_control_action = true;
  }

  // Handle roll maneuvers
  if (is_key_held(tKey::Q)) {
    FlightSystem::RollLeft(state, dt);

    is_issuing_control_action = true;
  } else if (is_key_held(tKey::E)) {
    FlightSystem::RollRight(state, dt);

    is_issuing_control_action = true;
  }

  // Handle auto-prograde actions
  if (did_press_key(tKey::SHIFT)) {
    state.flight_mode = FlightMode::AUTO_PROGRADE;
    state.ship_rotate_to_target_speed = 0.f;
  }

  // Handle auto-retrograde actions
  if (did_press_key(tKey::SPACE)) {
    state.flight_mode = FlightMode::AUTO_RETROGRADE;
    state.ship_rotate_to_target_speed = 0.f;
  }

  // Handle auto-docking actions
  if (did_press_key(tKey::ENTER)) {
    AttemptDockingProcedure(state);
    state.ship_rotate_to_target_speed = 0.f;
  }

  // Allow the ship to swivel quickly in automatic flight modes
  if (
    state.flight_mode == FlightMode::AUTO_PROGRADE ||
    state.flight_mode == FlightMode::AUTO_RETROGRADE
  ) {
    state.ship_rotate_to_target_speed += 2.f * dt;
  }

  if (
    state.flight_mode == FlightMode::AUTO_DOCK &&
    state.auto_dock_stage < AutoDockStage::APPROACH
  ) {
    state.ship_rotate_to_target_speed += 1.f * dt;
  }

  // Allow the ship to rotate to the camera orientation faster
  // the closer it is to the camera view's forward direction.
  // Rotate-to-camera speed values > 1 are reduced the more
  // the camera is pointed away from the ship direction.
  // This prevents rolling from being used as an exploit to
  // turn the ship around more quickly, since rolling ordinarily
  // rotates the ship faster to keep up with the camera
  // (necessary to reduce motion sickness).
  {
    float forward_alignment = tVec3f::dot(state.ship_rotation_basis.forward, state.view_forward_direction);
    if (forward_alignment < 0.f) forward_alignment = 0.f;

    if (state.ship_pitch_factor == 0.f) {
      // Only do exponential tapering when not pitching.
      // When pitching, we want to allow the rotate-to-target
      // value to be mostly preserved.
      forward_alignment = powf(forward_alignment, 20.f);

      if (state.ship_rotate_to_target_speed > 1.f) {
        state.ship_rotate_to_target_speed = Lerpf(state.ship_rotate_to_target_speed, 1.f, 1.f - forward_alignment);
      }
    }
  }

  state.ship_rotate_to_target_speed *= (1.f - dt);
  state.ship_position += state.ship_velocity * dt;

  // When panning the camera around while not issuing ship controls,
  // rapidly slow the ship's natural rotation drift
  if (
    state.flight_mode == FlightMode::MANUAL_CONTROL &&
    !is_issuing_control_action &&
    (tachyon->mouse_delta_x != 0 || tachyon->mouse_delta_y != 0)
  ) {
    state.ship_rotate_to_target_speed *= (1.f - 5.f * dt);
  }
}

const float AUTO_DOCK_APPROACH_SPEED = 1500.f;
const float AUTO_DOCK_APPROACH_SPEED_LIMIT = 2000.f;

static void HandleAutopilot(Tachyon* tachyon, State& state, const float dt) {
  switch (state.flight_mode) {
    case FlightMode::AUTO_RETROGRADE: {
      // Figure out how 'backward' the ship is pointed
      float reverse_dot = tVec3f::dot(state.ship_rotation_basis.forward, state.ship_velocity_basis.forward);

      if (reverse_dot < -0.f) {
        // Use the current speed to determine how much we need to accelerate in the opposite direction
        float acceleration = state.ship_velocity.magnitude() / 2.f;
        if (acceleration > 5000.f) acceleration = 5000.f;
        if (acceleration < 500.f) acceleration = 500.f;

        // Increase acceleration the more the ship is aligned with the 'backward' vector
        float speed = acceleration * powf(-reverse_dot, 15.f);

        FlightSystem::ThrustForward(state, dt, speed);
      }

      if (state.ship_velocity.magnitude() < 200.f) {
        // Restore manual control when sufficiently decelerated
        state.flight_mode = FlightMode::MANUAL_CONTROL;
      }

      break;
    }

    case FlightMode::AUTO_DOCK: {
      if (state.auto_dock_stage == AutoDockStage::APPROACH_DECELERATION) {
        float backward_alignment = tVec3f::dot(state.ship_rotation_basis.forward, state.ship_velocity_basis.forward.invert());
        if (backward_alignment < 0.f) backward_alignment = 0.f;

        float deceleration_alpha = backward_alignment * backward_alignment * backward_alignment;
        float deceleration_factor = Lerpf(0.f, 2000.f, deceleration_alpha);

        state.ship_rotate_to_target_speed += 0.5f * dt;

        FlightSystem::ThrustForward(state, dt, deceleration_factor);

        if (state.ship_velocity.magnitude() < 50.f) {
          state.auto_dock_stage = AutoDockStage::APPROACH_ALIGNMENT;
          state.ship_rotate_to_target_speed = 0.f;
        }
      }

      if (
        state.auto_dock_stage == AutoDockStage::APPROACH &&
        state.ship_velocity.magnitude() < AUTO_DOCK_APPROACH_SPEED_LIMIT
      ) {
        FlightSystem::ThrustForward(state, dt, AUTO_DOCK_APPROACH_SPEED);
      }
    }
  }
}

static void HandleFlightCamera(Tachyon* tachyon, State& state, const float dt) {
  auto& camera = tachyon->scene.camera;
  auto& meshes = state.meshes;
  float camera_lerp_speed_factor = 10.f;

  // Clamp roll speed
  if (state.camera_roll_speed > 3.f) state.camera_roll_speed = 3.f;
  if (state.camera_roll_speed < -3.f) state.camera_roll_speed = -3.f;

  state.camera_roll_speed *= (1.f - dt);

  if (is_window_focused()) {
    Quaternion turn = (
      Quaternion::fromAxisAngle(RIGHT_VECTOR, (float)tachyon->mouse_delta_y / 1000.f) *
      Quaternion::fromAxisAngle(UP_VECTOR, (float)tachyon->mouse_delta_x / 1000.f) *
      Quaternion::fromAxisAngle(FORWARD_VECTOR, state.camera_roll_speed * dt)
    );

    state.target_camera_rotation *= turn;

    // Swiveling
    {
      if (is_key_held(tKey::A)) {
        state.target_camera_rotation = state.target_camera_rotation * Quaternion::fromAxisAngle(state.ship_rotation_basis.up, -1.f * dt);
      } else if (is_key_held(tKey::D)) {
        state.target_camera_rotation = state.target_camera_rotation * Quaternion::fromAxisAngle(state.ship_rotation_basis.up, 1.f * dt);
      }
    }

    // Zoom in/out
    {
      if (did_wheel_down()) {
        state.ship_camera_distance_target += 1000.f;
      } else if (did_wheel_up()) {
        state.ship_camera_distance_target -= 1000.f;
        if (state.ship_camera_distance_target < 1200.f) state.ship_camera_distance_target = 1200.f;
      }
    }

    state.target_camera_rotation = state.target_camera_rotation.unit();
  }

  if (
    state.flight_mode == FlightMode::AUTO_PROGRADE ||
    state.flight_mode == FlightMode::AUTO_RETROGRADE || (
      state.flight_mode == FlightMode::AUTO_DOCK &&
      state.auto_dock_stage == AutoDockStage::APPROACH_ALIGNMENT &&
      GetTargetPositionAim(state, GetDockingPosition(tachyon, state)) > 0.5f
    )
  ) {
    // Gradually move the camera behind the player ship
    state.target_camera_rotation = Quaternion::slerp(
      state.target_camera_rotation,
      objects(meshes.hull)[0].rotation.opposite(),
      2.f * dt
    );
  }

  if (state.ship_pitch_factor != 0.f) {
    auto new_target =
      Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), -20.f * state.ship_pitch_factor * dt) *
      objects(meshes.hull)[0].rotation.opposite();

    state.target_camera_rotation = Quaternion::slerp(
      state.target_camera_rotation,
      new_target,
      2.f * abs(state.ship_pitch_factor) * dt
    );
  }

  float camera_lerp_alpha = camera_lerp_speed_factor * dt;
  if (camera_lerp_alpha > 1.f) camera_lerp_alpha = 1.f;

  camera.rotation = Quaternion::slerp(camera.rotation, state.target_camera_rotation, camera_lerp_alpha);

  UpdateViewDirections(tachyon, state);

  state.ship_camera_distance = Lerpf(state.ship_camera_distance, state.ship_camera_distance_target, 5.f * dt);

  float ship_speed = state.ship_velocity.magnitude();
  float speed_zoom_ratio = ship_speed / (ship_speed + 5000.f);
  float camera_radius = state.ship_camera_distance + 250.f * speed_zoom_ratio;

  camera.fov = 45.f + 10.f * speed_zoom_ratio;
  camera.position = state.ship_position - state.view_forward_direction * camera_radius + state.view_up_direction * 300.f;
}

// @todo move to HUDSystem
static void HandleFlightArrows(Tachyon* tachyon, State& state, const float dt) {
  const static float MIN_SPAWN_DISTANCE = 5000.f;
  const static float MAX_SPAWN_DISTANCE = 20000.f;

  auto& meshes = state.meshes;
  float speed = state.ship_velocity.magnitude();

  if (speed == 0.f) {
    return;
  }

  // Reduce distance to next flight path node spawn
  state.flight_path_spawn_distance_remaining -= speed * dt;

  // Move incoming flight path nodes toward the ship
  auto& flight_path = state.incoming_flight_path;
  tVec3f sideways = tVec3f::cross(state.ship_velocity_basis.forward, state.ship_rotation_basis.up).unit();
  tVec3f bottom_offset = state.ship_rotation_basis.up * -750.f;
  tVec3f left_offset = sideways * -750.f;
  tVec3f right_offset = sideways * 750.f;

  // Reset arrows each frame
  for (auto& arrow : objects(meshes.hud_flight_arrow)) {
    arrow.scale = 0.f;
    arrow.rotation = LookRotation(state.ship_velocity_basis.forward, state.ship_rotation_basis.up);

    commit(arrow);
  }

  float speed_ratio = state.ship_velocity.magnitude() / MAX_SHIP_SPEED;

  // Recalculate/reposition visible arrows
  for (int32 i = flight_path.size() - 1; i >= 0; i--) {
    auto& node = flight_path[i];
    auto direction_to_ship = (state.ship_position - node.position).unit();

    node.distance -= speed * dt;

    // Gradually curve nodes onto the updated flight path as it changes in real-time.
    // Determine the node's "progress" toward the ship, and blend between its original
    // spawn position and a position directly forward along the ship's trajectory.
    float target_distance = Lerpf(MAX_SPAWN_DISTANCE, node.spawn_distance, speed_ratio);
    float alpha = 1.f - node.distance / target_distance;
    tVec3f velocity_position = state.ship_position + state.ship_velocity_basis.forward * node.distance;

    node.position = Lerpf(node.spawn_position, velocity_position, alpha);

    float velocity_alignment = tVec3f::dot(direction_to_ship, state.ship_velocity_basis.forward);
    float forward_alignment = tVec3f::dot(direction_to_ship, state.ship_rotation_basis.forward);

    if (
      node.distance <= 0.f ||
      velocity_alignment > 0.f ||
      forward_alignment > 0.f
    ) {
      flight_path.erase(flight_path.begin() + i);

      continue;
    }

    float path_progress = 1.f - node.distance / node.spawn_distance;
    float brightness = 1.f;
    if (path_progress < 0.1f) brightness = path_progress / 0.1f;
    if (path_progress > 0.8f) brightness = 1.f - (path_progress - 0.8f) / 0.2f;

    if (forward_alignment > -0.1f) {
      brightness *= (forward_alignment * -10.f);
    }

    auto& arrow_1 = objects(meshes.hud_flight_arrow)[i * 2];
    auto& arrow_2 = objects(meshes.hud_flight_arrow)[i * 2 + 1];

    arrow_1.position = node.position + bottom_offset + left_offset;
    arrow_2.position = node.position + bottom_offset + right_offset;

    arrow_1.scale = arrow_2.scale = 200.f;
    // @todo change color in auto-docking approach mode
    arrow_1.color = arrow_2.color = tVec4f(0.1f, 0.2f, 1.f, brightness);

    commit(arrow_1);
    commit(arrow_2);
  }

  // Add new flight path nodes
  if (state.flight_path_spawn_distance_remaining <= 0.f) {
    FlightPathNode node;

    node.spawn_distance = Lerpf(MIN_SPAWN_DISTANCE, MAX_SPAWN_DISTANCE, speed_ratio);
    node.position = state.ship_position + state.ship_velocity_basis.forward * node.spawn_distance;
    node.spawn_position = node.position;
    node.distance = node.spawn_distance;

    flight_path.push_back(node);

    state.flight_path_spawn_distance_remaining = node.spawn_distance * 0.25f;

    state.flight_arrow_cycle_step++;
    if (state.flight_arrow_cycle_step > 4) state.flight_arrow_cycle_step = 0;
  }
}

static void UpdateShipVelocityBasis(State& state) {
  auto forward = state.ship_velocity.unit();
  auto up = UP_VECTOR;
  auto sideways = tVec3f::cross(forward, up).unit();

  up = tVec3f::cross(sideways, forward);

  state.ship_velocity_basis.forward = forward;
  state.ship_velocity_basis.up = up;
  state.ship_velocity_basis.sideways = sideways;
}

static void HandlePlayerDrone(Tachyon* tachyon, State& state, const float dt) {
  auto& camera = tachyon->scene.camera;
  auto& meshes = state.meshes;

  auto& hull = objects(meshes.hull)[0];
  auto& streams = objects(meshes.streams)[0];
  auto& thrusters = objects(meshes.thrusters)[0];
  auto& trim = objects(meshes.trim)[0];

  auto target_rotation = camera.rotation.opposite();

  if (state.ship_velocity.magnitude() > 0.f) {
    UpdateShipVelocityBasis(state);
  }

  if (state.ship_pitch_factor != 0.f) {
    float angle_delta = 50.f * state.ship_pitch_factor * dt;

    target_rotation =
      target_rotation *
      Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), angle_delta);

    FlightSystem::HandlePitch(state, dt);
  }

  // @todo move to HandleAutopilot()
  if (state.flight_mode == FlightMode::AUTO_PROGRADE) {
    target_rotation = DirectionToQuaternion(state.ship_velocity_basis.forward.invert());
  }

  // @todo move to HandleAutopilot()
  if (state.flight_mode == FlightMode::AUTO_RETROGRADE) {
    target_rotation = LookRotation(
      state.ship_velocity_basis.forward,
      state.ship_rotation_basis.up
    );
  }

  // @todo move to HandleAutopilot()
  if (state.flight_mode == FlightMode::AUTO_DOCK) {
    switch (state.auto_dock_stage) {
      case AutoDockStage::APPROACH_DECELERATION: {
        target_rotation = DirectionToQuaternion(state.ship_velocity_basis.forward);

        break;
      }

      case AutoDockStage::APPROACH_ALIGNMENT: {
        auto docking_position = GetDockingPosition(tachyon, state);
        // @todo use live object
        auto target_object_rotation = state.docking_target.rotation;
        auto forward = (docking_position - state.ship_position).unit();
        auto target_object_up = target_object_rotation.getUpDirection();

        target_rotation = LookRotation(forward.invert(), target_object_up);

        if (GetTargetPositionAim(state, docking_position) > 0.99999f) {
          state.auto_dock_stage = AutoDockStage::APPROACH;
        } else {
          // @hack slow the ship down after deceleration
          state.ship_velocity *= (1.f - dt);
        }

        break;
      }

      case AutoDockStage::APPROACH: {
        auto docking_position = GetDockingPosition(tachyon, state);
        auto target_distance = (state.ship_position - docking_position).magnitude();

        if (state.ship_velocity.magnitude() >= AUTO_DOCK_APPROACH_SPEED_LIMIT && target_distance < 10000.f) {
          state.auto_dock_stage = AutoDockStage::DOCKING;
        }

        break;
      }

      case AutoDockStage::DOCKING: {
        auto docking_position = GetDockingPosition(tachyon, state);
        auto target_distance = (state.ship_position - docking_position).magnitude();

        target_rotation =
          // @todo use live object
          state.docking_target.rotation *
          Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), t_PI);

        state.ship_rotate_to_target_speed = 0.3f;

        if (target_distance < 5000.f) {
          float speed = AUTO_DOCK_APPROACH_SPEED_LIMIT * (target_distance / 5000.f);

          state.ship_velocity = state.ship_velocity_basis.forward * speed;
        }

        if (target_distance < 250.f) {
          state.auto_dock_stage = AutoDockStage::DOCKED;
          state.ship_velocity = 0.f;
          state.ship_rotate_to_target_speed = 0.f;
          state.ship_camera_distance_target = 30000.f;

          state.target_camera_rotation =
            Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), 0.6f) *
            Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), t_PI * 1.2f) *
            state.docking_target.rotation.opposite();
        }
      }
    }
  }

  // @todo will nlerp work here?
  auto rotation = Quaternion::slerp(hull.rotation, target_rotation, state.ship_rotate_to_target_speed * dt);

  hull.position = state.ship_position;
  streams.position = state.ship_position;
  thrusters.position = state.ship_position;
  trim.position = state.ship_position;

  hull.rotation = streams.rotation = thrusters.rotation = trim.rotation = rotation;

  commit(hull);
  commit(streams);
  commit(thrusters);
  commit(trim);

  state.ship_rotation_basis.forward = rotation.getDirection();
  state.ship_rotation_basis.up = rotation.getUpDirection();
  state.ship_rotation_basis.sideways = rotation.getLeftDirection().invert();
}

// @todo move to debug.cpp
static void UpdateOrthonormalBasisDebugVectors(
  Tachyon* tachyon,
  State& state,
  tObject& forward,
  tObject& sideways,
  tObject& up,
  const OrthonormalBasis& basis,
  const float thickness,
  const float length
) {
  forward.position = state.ship_position + basis.forward * length;
  sideways.position = state.ship_position + basis.sideways * length;
  up.position = state.ship_position + basis.up * length;

  forward.rotation = DirectionToQuaternion(basis.forward);
  sideways.rotation = DirectionToQuaternion(basis.sideways);
  up.rotation = DirectionToQuaternion(basis.up);

  forward.scale = tVec3f(thickness, thickness, length);
  sideways.scale = tVec3f(thickness, thickness, length);
  up.scale = tVec3f(thickness, thickness, length);

  commit(forward);
  commit(sideways);
  commit(up);
}

// @todo move to debug.cpp
static void UpdateShipDebugVectors(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  {
    auto& forward = objects(meshes.cube)[0];
    auto& sideways = objects(meshes.cube)[1];
    auto& up = objects(meshes.cube)[2];

    forward.color = tVec4f(1.f, 0, 0, 1.f);
    sideways.color = tVec4f(0, 1.f, 0, 1.f);
    up.color = tVec4f(0, 0, 1.f, 1.f);

    UpdateOrthonormalBasisDebugVectors(tachyon, state, forward, sideways, up, state.ship_velocity_basis, 8.f, 250.f);
  }

  {
    auto& forward = objects(meshes.cube)[3];
    auto& sideways = objects(meshes.cube)[4];
    auto& up = objects(meshes.cube)[5];

    forward.color = tVec4f(1.f, 1.f, 0, 1.f);
    sideways.color = tVec4f(0, 1.f, 1.f, 1.f);
    up.color = tVec4f(1.f, 0, 1.f, 1.f);

    UpdateOrthonormalBasisDebugVectors(tachyon, state, forward, sideways, up, state.ship_rotation_basis, 6.f, 200.f);
  }
}

static void ShowDevLabels(Tachyon* tachyon, State& state) {
  auto& camera = tachyon->scene.camera;
  auto& meshes = state.meshes;
  auto& hull = objects(meshes.hull)[0];

  add_dev_label("Game time", std::to_string(state.current_game_time));
  add_dev_label("Ship position", state.ship_position.toString());
  add_dev_label("Ship velocity", state.ship_velocity.toString());
  add_dev_label("Ship speed", std::to_string(state.ship_velocity.magnitude()));
  add_dev_label("Camera position", camera.position.toString());
  add_dev_label("Camera rotation", camera.rotation.toString());
}

void Cosmodrone::StartGame(Tachyon* tachyon) {
  MeshLibrary::LoadMeshes(tachyon, state);
  WorldSetup::InitializeGameWorld(tachyon, state);
  Editor::InitializeEditor(tachyon, state);

  // @todo UI::Initialize()
  {
    state.ui.target_indicator = Tachyon_CreateUIElement("./cosmodrone/assets/ui/target.png");
    state.ui.zone_target_indicator = Tachyon_CreateUIElement("./cosmodrone/assets/ui/zone-target.png");
    state.ui.selected_target_corner = Tachyon_CreateUIElement("./cosmodrone/assets/ui/selected-target-corner.png");
    state.ui.selected_target_center = Tachyon_CreateUIElement("./cosmodrone/assets/ui/selected-target-center.png");
  }
}

void Cosmodrone::UpdateGame(Tachyon* tachyon, const float dt) {
  auto& camera = tachyon->scene.camera;
  auto& meshes = state.meshes;

  tachyon->scene.transform_origin = camera.position;

  // @todo dev mode only
  if (did_press_key(tKey::NUM_1)) {
    if (state.is_editor_active) {
      Editor::DisableEditor(tachyon, state);
    } else {
      Editor::EnableEditor(tachyon, state);
    }
  }

  // @todo dev mode only
  objects(meshes.cube).disabled = state.is_editor_active || !tachyon->show_developer_tools;
  objects(meshes.hud_flight_arrow).disabled = state.is_editor_active;
  objects(meshes.hud_wedge).disabled = state.is_editor_active;

  // @todo dev mode only
  if (state.is_editor_active) {
    Editor::HandleEditor(tachyon, state, dt);

    return;
  }

  // @todo dev mode only
  objects(state.meshes.editor_guideline).disabled = true;

  HandleFlightControls(tachyon, state, dt);
  HandleAutopilot(tachyon, state, dt);
  HandleFlightCamera(tachyon, state, dt);
  HandleFlightArrows(tachyon, state, dt);
  HandlePlayerDrone(tachyon, state, dt);

  HUDSystem::HandleHUD(tachyon, state, dt);
  TargetSystem::HandleTargetTrackers(tachyon, state, dt);
  WorldBehavior::UpdateWorld(tachyon, state, dt);

  // @todo dev mode only
  if (tachyon->show_developer_tools) {
    UpdateShipDebugVectors(tachyon, state);
    ShowDevLabels(tachyon, state);
  }
}