#include "cosmodrone/autopilot.h"
#include "cosmodrone/drone_flight_system.h"
#include "cosmodrone/flight_system_delegator.h"
#include "cosmodrone/game.h"
#include "cosmodrone/game_editor.h"
#include "cosmodrone/game_types.h"
#include "cosmodrone/hud_system.h"
#include "cosmodrone/mesh_library.h"
#include "cosmodrone/piloting.h"
#include "cosmodrone/target_system.h"
#include "cosmodrone/utilities.h"
#include "cosmodrone/world_behavior.h"
#include "cosmodrone/world_setup.h"

using namespace Cosmodrone;

// @todo move to constants
const static auto FORWARD_VECTOR = tVec3f(0, 0, -1.f);
const static auto UP_VECTOR = tVec3f(0, 1.f, 0);
const static auto RIGHT_VECTOR = tVec3f(1.f, 0, 0);

// @todo pass into StartGame() and RunGame() from main.cpp
static State state;

// @todo remove in favor of LookRotation()
static Quaternion DirectionToQuaternion(const tVec3f& direction) {
  auto yaw = atan2f(direction.x, direction.z);
  auto pitch = atan2f(direction.xz().magnitude(), direction.y) - t_HALF_PI;

  return (
    Quaternion::fromAxisAngle(UP_VECTOR, yaw) *
    Quaternion::fromAxisAngle(RIGHT_VECTOR, pitch)
  );
}

static void UpdateViewDirections(Tachyon* tachyon, State& state, const float dt) {
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

  // @todo cleanup
  if (
    state.reticle_view_forward.x == 0.f &&
    state.reticle_view_forward.y == 0.f &&
    state.reticle_view_forward.z == 0.f
  ) {
    state.reticle_view_forward = state.view_forward_direction;
  } else {
    state.reticle_view_forward = tVec3f::lerp(
      state.reticle_view_forward,
      state.view_forward_direction,
      5.f * dt
    ).unit();
  }
}

static void HandleInputs(Tachyon* tachyon, State& state, const float dt) {
  bool is_issuing_control_action = false;

  // Handle forward action
  if (is_key_held(tKey::W)) {
    FlightSystemDelegator::Forward(state, dt);

    is_issuing_control_action = true;
  } else {
    // Reset duration when not holding forward
    state.controlled_thrust_duration = 0.f;
  }

  // Slow the ship down when not fully accelerating in FIGHTER mode
  if (
    state.flight_system == FlightSystem::FIGHTER &&
    state.controlled_thrust_duration < 1.f &&
    state.ship_velocity.magnitude() > 500.f
  ) {
    state.ship_velocity *= 1.f - 1.3f * dt;
  }

  // Handle pull-back actions
  if (is_key_held(tKey::S)) {
    FlightSystemDelegator::PullBack(state, dt, 1.f);

    is_issuing_control_action = true;
  } else {
    // Reduce pitch gradually
    state.ship_pitch_factor *= (1.f - 2.f * dt);

    // Prevent a long reduction tail and just snap to 0
    if (abs(state.ship_pitch_factor) < 0.01f) {
      state.ship_pitch_factor = 0.f;
    }
  }

  // Handle left/right actions
  if (is_key_held(tKey::A)) {
    FlightSystemDelegator::Left(state, dt);

    is_issuing_control_action = true;
  } else if (is_key_held(tKey::D)) {
    FlightSystemDelegator::Right(state, dt);

    is_issuing_control_action = true;
  }

  // Handle roll actions
  if (is_key_held(tKey::Q)) {
    FlightSystemDelegator::RollLeft(state, dt);

    is_issuing_control_action = true;
  } else if (is_key_held(tKey::E)) {
    FlightSystemDelegator::RollRight(state, dt);

    is_issuing_control_action = true;
  }

  // Handle auto-prograde actions
  if (did_press_key(tKey::SHIFT)) {
    FlightSystemDelegator::AutoPrograde(state, dt);
  }

  // Handle auto-stop actions
  if (did_press_key(tKey::SPACE)) {
    FlightSystemDelegator::AutoStop(state, dt);

    // @todo factor
    if (state.flight_system == FlightSystem::FIGHTER) {
      // Stop camera drift when activating a fighter quick-target/quick-reverse
      state.target_camera_rotation = tachyon->scene.camera.rotation;
    }
  }

  // Handle auto-dock/undock actions
  if (did_press_key(tKey::ENTER)) {
    FlightSystemDelegator::DockOrUndock(tachyon, state, dt);
  }

  if (did_press_key(tKey::F)) {
    state.photo_mode = !state.photo_mode;
  }

  // @todo How does any of the below have to do with inputs? Why is it here?
  // This stuff should be moved to HandleDrone() or some intermediate step.

  // @todo move to autopilot.cpp
  if (
    state.flight_mode == FlightMode::AUTO_DOCK &&
    state.auto_dock_stage < AutoDockStage::APPROACH
  ) {
    state.ship_rotate_to_target_speed += 2.f * dt;
  }

  // Some complicated logic to ensure that the ship can rotate faster
  // when pointing forward along our view direction, but takes longer
  // to rotate to a totally different view direction. We don't want
  // the ship swinging around rapidly and allowing comically fast
  // turning under normal manual control circumstances. Only applies
  // during manual control; all bets are off in autopilot because
  // autopilot routines need direct control over ship/camera rotation.
  {
    if (!Autopilot::IsAutopilotActive(state)) {
      float forward_alignment = tVec3f::dot(state.ship_rotation_basis.forward, state.view_forward_direction);
      if (forward_alignment < 0.f) forward_alignment = 0.f;

      if (state.ship_pitch_factor == 0.f) {
        // Only do exponential tapering when not pitching.
        // When pitching, we want to allow the rotate-to-target
        // value to be mostly preserved.
        forward_alignment = powf(forward_alignment, 20.f);

        if (state.ship_rotate_to_target_speed > 1.f) {
          state.ship_rotate_to_target_speed = Tachyon_Lerpf(state.ship_rotate_to_target_speed, 1.f, 1.f - forward_alignment);
        }
      }
    }
  }

  // Enforce maximum ship speed
  {
    float ship_speed = state.ship_velocity.magnitude();
    float max_ship_speed = Utilities::GetMaxShipSpeed(state);

    if (ship_speed > max_ship_speed) {
      state.ship_velocity = state.ship_velocity.unit() * max_ship_speed;
    }
  }

  state.ship_rotate_to_target_speed *= (1.f - dt);
  state.ship_position += state.ship_velocity * dt;

  // When panning the camera around while not issuing ship controls,
  // rapidly slow the ship's natural rotation drift
  if (
    state.flight_mode == FlightMode::MANUAL_CONTROL &&
    !is_issuing_control_action &&
    (tachyon->mouse_delta_x != 0 || tachyon->mouse_delta_y != 0) &&
    // Only do this when the camera isn't behind the ship.
    // It looks a little odd when the ship suddenly stops
    // drifting while the camera is following it.
    tVec3f::dot(state.view_forward_direction, state.ship_rotation_basis.forward) < 0.95f
  ) {
    // state.ship_rotate_to_target_speed *= (1.f - 10.f * dt);
  }
}

// @todo camera_system.cpp
static void HandleCamera(Tachyon* tachyon, State& state, const float dt) {
  auto& camera = tachyon->scene.camera;
  auto& meshes = state.meshes;

  // Clamp roll speed
  if (state.camera_roll_speed > 3.f) state.camera_roll_speed = 3.f;
  if (state.camera_roll_speed < -3.f) state.camera_roll_speed = -3.f;

  float roll_speed_dampening_factor = state.is_piloting_vehicle ? 3.f : 1.f;

  state.camera_roll_speed *= (1.f - roll_speed_dampening_factor * dt);
  state.camera_yaw_speed *= (1.f - 5.f * dt);

  if (is_window_focused()) {
    float mouse_divisor =
      state.flight_system == FlightSystem::FIGHTER && state.controlled_thrust_duration > 0.f
        ? 1500.f
        : 1000.f;

    Quaternion turn = (
      Quaternion::fromAxisAngle(RIGHT_VECTOR, (float)tachyon->mouse_delta_y / mouse_divisor) *
      Quaternion::fromAxisAngle(UP_VECTOR, (float)tachyon->mouse_delta_x / mouse_divisor) *
      Quaternion::fromAxisAngle(FORWARD_VECTOR, state.camera_roll_speed * dt)
    );

    state.target_camera_rotation *= turn;

    // Swiveling
    {
      if (is_key_held(tKey::A)) {
        state.target_camera_rotation =
          state.target_camera_rotation *
          Quaternion::fromAxisAngle(state.ship_rotation_basis.up, -state.camera_yaw_speed * dt);

      } else if (is_key_held(tKey::D)) {
        state.target_camera_rotation =
          state.target_camera_rotation *
          Quaternion::fromAxisAngle(state.ship_rotation_basis.up, state.camera_yaw_speed * dt);
      }
    }

    // Zoom in/out
    {
      if (did_wheel_down()) {
        state.ship_camera_distance_target += 1000.f;
      } else if (did_wheel_up()) {
        state.ship_camera_distance_target -= 1000.f;
        if (state.ship_camera_distance_target < 1800.f) state.ship_camera_distance_target = 1800.f;
      }
    }

    state.target_camera_rotation = state.target_camera_rotation.unit();
  }

  // Handle fighter quick reversal camera
  if (
    state.flight_system == FlightSystem::FIGHTER &&
    state.flight_mode == FlightMode::AUTO_RETROGRADE
  ) {
    float reversal_duration = state.current_game_time - state.last_fighter_reversal_time;

    float blend_factor = reversal_duration - 0.1f;
    if (blend_factor > 1.f) blend_factor = 1.f;
    if (blend_factor < 0.f) blend_factor = 0.f;
    blend_factor = 100.f * powf(blend_factor, 2.f) * dt;
    if (blend_factor > 1.f) blend_factor = 1.f;

    state.target_camera_rotation = Quaternion::slerp(
      state.target_camera_rotation,
      objects(meshes.hull)[0].rotation.opposite(),
      blend_factor
    );
  }

  // Handle auto-orientation
  if (
    state.flight_system == FlightSystem::DRONE && (
    state.flight_mode == FlightMode::AUTO_PROGRADE ||
    state.flight_mode == FlightMode::AUTO_RETROGRADE || (
      state.flight_mode == FlightMode::AUTO_DOCK &&
      state.auto_dock_stage == AutoDockStage::APPROACH_ALIGNMENT &&
      Autopilot::GetDockingAlignment(state, Autopilot::GetDockingPosition(tachyon, state)) > 0.5f
    )
  )) {
    state.target_camera_rotation = Quaternion::slerp(
      state.target_camera_rotation,
      objects(meshes.hull)[0].rotation.opposite(),
      dt
    );
  }

  // Handle pitch changes
  if (state.ship_pitch_factor != 0.f) {
    auto new_target =
      Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), -0.5f * state.ship_pitch_factor) *
      camera.rotation;

    float blend_factor = state.flight_system == FlightSystem::FIGHTER
      ? 30.f * powf(abs(state.ship_pitch_factor), 4.f)
      : 2.f;

    state.target_camera_rotation = Quaternion::slerp(
      state.target_camera_rotation,
      new_target,
      blend_factor * abs(state.ship_pitch_factor) * dt
    );
  }

  // When charging up fighter ships, slide the target camera
  // behind the ship to cause the aim direction to slow rapidly
  if (
    state.flight_system == FlightSystem::FIGHTER &&
    (state.controlled_thrust_duration > 0.f && state.controlled_thrust_duration < 1.f)
  ) {
    state.target_camera_rotation = Quaternion::slerp(
      state.target_camera_rotation,
      objects(meshes.hull)[0].rotation.opposite(),
      2.f * dt
    );
  }

  float ship_speed = state.ship_velocity.magnitude();
  float speed_ratio = ship_speed / Utilities::GetMaxShipSpeed(state);
  float rate;

  if (state.flight_system == FlightSystem::FIGHTER) {
    if (state.current_game_time - state.last_fighter_reversal_time < 2.f) {
      float alpha = 5.f * (state.current_game_time - state.last_fighter_reversal_time);
      if (alpha > 1.f) alpha = 1.f;

      rate = 10.f * alpha;
    } else {
      rate = Tachyon_Lerpf(1.4f, 1.f, speed_ratio);
    }
  } else {
    rate = 2.f;
  }

  camera.rotation = Quaternion::slerp(camera.rotation, state.target_camera_rotation, rate * dt);

  UpdateViewDirections(tachyon, state, dt);

  float speed_zoom_ratio = ship_speed / (ship_speed + 5000.f);
  float boost_intensity;

  // Calculate thruster boost effect
  {
    state.camera_boost_intensity += 10.f * state.controlled_thrust_duration * dt;
    state.camera_boost_intensity *= 1.f - 10.f * dt;
    if (state.camera_boost_intensity > 1.f) state.camera_boost_intensity = 1.f;

    if (state.flight_system == FlightSystem::FIGHTER) {
      boost_intensity =
        state.controlled_thrust_duration < 0.25f
          ? -12.f * state.controlled_thrust_duration :
        state.controlled_thrust_duration < 1.f
          ? -3.f :
        3.f;

      // When stopping a fighter vehicle, snap the FoV back to a smaller
      // value and gradually transition to normal again
      if (ship_speed > 500.f && state.controlled_thrust_duration == 0.f) {
        float blend = powf(1.f - speed_ratio, 1.f / 3.f);

        boost_intensity = Tachyon_Lerpf(-10.f, 0.f, blend);
      }
    } else {
      boost_intensity =
        state.camera_boost_intensity < 0.4f
          ? sqrtf(state.camera_boost_intensity * 2.5f)
          : 1.f;
    }
  }

  // Set the field of view
  {
    state.target_camera_fov =
      45.f +
      5.f * boost_intensity +
      5.f * state.ship_pitch_factor;

    if (state.flight_mode != FlightMode::AUTO_DOCK || state.auto_dock_stage < AutoDockStage::APPROACH) {
      // Increase FoV proportional to speed when not doing docking auto-approach
      state.target_camera_fov += 10.f * speed_zoom_ratio;
    }

    if (state.flight_system == FlightSystem::FIGHTER) {
      // Bump up the FoV when flying fighter vehicles at high speed
      state.target_camera_fov += 10.f * speed_ratio;
    }

    float fov_blend_factor = (
      state.flight_system == FlightSystem::FIGHTER &&
      state.controlled_thrust_duration > 0.f
    ) ? 10.f : 2.f;

    camera.fov = Tachyon_Lerpf(camera.fov, state.target_camera_fov, fov_blend_factor * dt);
  }

  // Set the camera position
  {
    float target_distance =
      state.ship_camera_distance_target +
      250.f * speed_zoom_ratio +
      (state.is_piloting_vehicle ? -6000.f * speed_zoom_ratio : 0.f);

    state.ship_camera_distance = Tachyon_Lerpf(state.ship_camera_distance, target_distance, 5.f * dt);

    // @todo make a utility for this
    if (state.flight_system == FlightSystem::FIGHTER) {
      if (state.current_piloted_vehicle.root_object.mesh_index == meshes.freight_dock) {
        state.camera_up_distance = Tachyon_Lerpf(state.camera_up_distance, 4000.f, dt);
      } else {
        state.camera_up_distance = Tachyon_Lerpf(state.camera_up_distance, 3500.f, dt);
      }
    } else {
      state.camera_up_distance = Tachyon_Lerpf(state.camera_up_distance, 500.f, 5.f * dt);
    }

    // @todo orthonormal basis for view
    tVec3f sideways = tVec3f::cross(state.view_up_direction, state.view_forward_direction);

    float max_side_distance = state.flight_system == FlightSystem::FIGHTER ? 900.f : 300.f;

    state.camera_side_distance = Tachyon_Lerpf(state.camera_side_distance, -state.banking_factor * max_side_distance, dt);

    auto& reference_position = state.is_piloting_vehicle
      ? get_live_object(state.docking_target)->position
      : state.ship_position;

    camera.position =
      reference_position -
      state.view_forward_direction * state.ship_camera_distance +
      state.view_up_direction * state.camera_up_distance +
      state.view_up_direction * abs(state.camera_side_distance) +
      sideways * state.camera_side_distance;
  }
}

// @todo maybe we should just get rid of this
static void HandleFlightGuides(Tachyon* tachyon, State& state, const float dt) {
  const static float MIN_SPAWN_DISTANCE = 5000.f;

  // @todo make a helper for this
  const float max_spawn_distance =
    state.is_piloting_vehicle
      ? 100000.f :
    20000.f;

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

  // Reset guidance objects each frame
  for (auto& arrow : objects(meshes.hud_flight_arrow)) {
    arrow.scale = 0.f;
    arrow.rotation = Quaternion::FromDirection(state.ship_velocity_basis.forward, state.ship_rotation_basis.up);

    commit(arrow);
  }

  for (auto& curve : objects(meshes.hud_flight_curve)) {
    curve.scale = 0.f;
    curve.rotation = Quaternion::FromDirection(state.ship_velocity_basis.forward, state.ship_rotation_basis.up);

    commit(curve);
  }

  // @todo consider removing this feature altogether
  return;

  float speed_ratio = state.ship_velocity.magnitude() / Utilities::GetMaxShipSpeed(state);

  float brightness_factor =
    state.is_piloting_vehicle ? (
      speed_ratio > 0.5f ? 2.f * (speed_ratio - 0.5f) : 0.f
    ) : 0.5f;

  // @todo make a helper for these
  tVec3f guide_color =
    Autopilot::IsDoingDockingApproach(state)
      ? tVec3f(1.f, 0.6f, 0.2f) :
    state.is_piloting_vehicle
      ? tVec3f(1.f, 0.5f, 0.2f) :
    tVec3f(0.1f, 1.f, 0.7f);

  float guide_scale =
    state.is_piloting_vehicle
      ? 10000.f
      : 200.f;

  uint16 guide_mesh =
    state.is_piloting_vehicle
      ? meshes.hud_flight_curve
      : meshes.hud_flight_arrow;

  float guide_offset =
    state.is_piloting_vehicle
      ? 10000.f
      : 750.f;

  tVec3f bottom_offset = state.ship_rotation_basis.up * -guide_offset;

  if (state.is_piloting_vehicle) {
    bottom_offset = tVec3f(0.f);
  }

  tVec3f left_offset = sideways * -guide_offset;
  tVec3f right_offset = sideways * guide_offset;

  // Recalculate/reposition visible arrows
  for (int32 i = flight_path.size() - 1; i >= 0; i--) {
    auto& node = flight_path[i];
    auto direction_to_ship = (state.ship_position - node.position).unit();

    node.distance -= speed * dt;

    // Gradually curve nodes onto the updated flight path as it changes in real-time.
    // Determine the node's "progress" toward the ship, and blend between its original
    // spawn position and a position directly forward along the ship's trajectory.
    float alpha = 1.f - node.distance / node.spawn_distance;
    tVec3f velocity_position = state.ship_position + state.ship_velocity_basis.forward * node.distance;

    node.position = tVec3f::lerp(node.spawn_position, velocity_position, alpha);

    float velocity_alignment = tVec3f::dot(direction_to_ship, state.ship_velocity_basis.forward);
    float forward_alignment = tVec3f::dot(direction_to_ship, state.ship_rotation_basis.forward);

    if (
      node.distance <= 0.f ||
      velocity_alignment > 0.f ||
      forward_alignment > 0.f ||
      speed_ratio < 0.1f ||
      Autopilot::IsDoingDockingAlignment(state)
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

    brightness *= brightness_factor;

    auto& guide_1 = objects(guide_mesh)[i * 2];
    auto& guide_2 = objects(guide_mesh)[i * 2 + 1];

    guide_1.position = node.position + bottom_offset + left_offset;
    guide_2.position = node.position + bottom_offset + right_offset;

    guide_1.scale =
    guide_2.scale = guide_scale;

    guide_1.color =
    guide_2.color = tVec4f(guide_color, brightness);

    guide_2.rotation *= Quaternion::fromAxisAngle(state.ship_velocity_basis.forward, t_PI);

    // @experimental
    if (state.photo_mode) {
      guide_1.scale = guide_2.scale = 0.f;
    }

    commit(guide_1);
    commit(guide_2);
  }

  // Add new flight path nodes
  if (state.flight_path_spawn_distance_remaining <= 0.f) {
    FlightPathNode node;

    node.spawn_distance = Tachyon_Lerpf(MIN_SPAWN_DISTANCE, max_spawn_distance, speed_ratio);
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

static void HandleShipBanking(Tachyon* tachyon, State& state) {
  auto& hull = objects(state.meshes.hull)[0];
  auto target_left = state.target_ship_rotation.getLeftDirection();

  float left_dot = tVec3f::dot(state.ship_rotation_basis.forward, target_left);
  if (left_dot < 0.f) left_dot = 0.f;

  float right_dot = tVec3f::dot(state.ship_rotation_basis.forward, target_left.invert());
  if (right_dot < 0.f) right_dot = 0.f;

  if (abs(left_dot) > abs(right_dot)) {
    state.banking_factor = -left_dot;
  } else {
    state.banking_factor = right_dot;
  }

  if (state.flight_system == FlightSystem::FIGHTER) {
    // Increase the amount of banking when flying fighter ships
    float speed_ratio = state.ship_velocity.magnitude() / Utilities::GetMaxShipSpeed(state);

    state.banking_factor *= (speed_ratio + 0.25f) * 1.2f;
  }

  if (state.banking_factor > 1.5f) state.banking_factor = 1.5f;
  if (state.banking_factor < -1.5f) state.banking_factor = -1.5f;

  float multiplier = state.ship_rotate_to_target_speed;
  if (multiplier > 1.f) multiplier = 1.f;

  state.banking_factor *= multiplier;

  state.target_ship_rotation =
    state.target_ship_rotation *
    Quaternion::fromAxisAngle(tVec3f(0, 0, 1.f), state.banking_factor);
}

static void HandleDrone(Tachyon* tachyon, State& state, const float dt) {
  auto& camera = tachyon->scene.camera;
  auto& meshes = state.meshes;

  auto& hull = objects(meshes.hull)[0];
  auto& streams = objects(meshes.streams)[0];
  auto& thrusters = objects(meshes.thrusters)[0];
  auto& trim = objects(meshes.trim)[0];
  auto& jets = objects(meshes.jets)[0];

  if (state.ship_velocity.magnitude() > 0.f) {
    UpdateShipVelocityBasis(state);
  }

  if (!Autopilot::IsAutopilotActive(state)) {
    state.target_ship_rotation = camera.rotation.opposite();

    if (state.ship_pitch_factor != 0.f) {
      FlightSystemDelegator::HandlePitch(state, dt);
    }
  }

  HandleShipBanking(tachyon, state);

  // @todo will nlerp work here?
  auto rotation = Quaternion::slerp(hull.rotation, state.target_ship_rotation, state.ship_rotate_to_target_speed * dt);

  if (Autopilot::IsDocked(state)) {
    state.ship_position = tVec3f::lerp(state.ship_position, state.docking_position, dt);
  }

  hull.position =
  streams.position =
  thrusters.position =
  trim.position =
  jets.position = state.ship_position;

  hull.rotation =
  streams.rotation =
  thrusters.rotation =
  trim.rotation =
  jets.rotation = rotation;

  hull.scale =
  streams.scale =
  thrusters.scale =
  trim.scale =
  jets.scale = state.photo_mode ? 0.f : 600.f;

  // Gradually taper jets intensity/update visibility
  {
    state.jets_intensity *= 1.f - 3.f * dt;

    if (state.jets_intensity < 0.f) state.jets_intensity = 0.f;
    if (state.jets_intensity > 1.f) state.jets_intensity = 1.f;

    jets.color = tVec4f(0.1f, 0.2f, 1.f, state.jets_intensity);
  }

  commit(hull);
  commit(streams);
  commit(thrusters);
  commit(trim);
  commit(jets);

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

  // Velocity
  {
    auto& forward = objects(meshes.cube)[0];
    auto& sideways = objects(meshes.cube)[1];
    auto& up = objects(meshes.cube)[2];

    forward.color = tVec4f(1.f, 0, 0, 1.f);
    sideways.color = tVec4f(0, 1.f, 0, 1.f);
    up.color = tVec4f(0, 0, 1.f, 1.f);

    UpdateOrthonormalBasisDebugVectors(tachyon, state, forward, sideways, up, state.ship_velocity_basis, 15.f, 500.f);
  }

  // Rotation
  {
    auto& forward = objects(meshes.cube)[3];
    auto& sideways = objects(meshes.cube)[4];
    auto& up = objects(meshes.cube)[5];

    forward.color = tVec4f(1.f, 1.f, 0, 1.f);
    sideways.color = tVec4f(0, 1.f, 1.f, 1.f);
    up.color = tVec4f(1.f, 0, 1.f, 1.f);

    UpdateOrthonormalBasisDebugVectors(tachyon, state, forward, sideways, up, state.ship_rotation_basis, 12.f, 400.f);
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
  WorldSetup::InitWorld(tachyon, state);
  Editor::InitializeEditor(tachyon, state);

  // @todo UI::Initialize()
  {
    state.ui.drone_reticle = Tachyon_CreateUIElement("./cosmodrone/assets/ui/drone-reticle.png");
    state.ui.fighter_reticle_frame = Tachyon_CreateUIElement("./cosmodrone/assets/ui/fighter-reticle-frame.png");
    state.ui.fighter_reticle_center = Tachyon_CreateUIElement("./cosmodrone/assets/ui/fighter-reticle-center.png");
    state.ui.fighter_reticle_blinker = Tachyon_CreateUIElement("./cosmodrone/assets/ui/fighter-reticle-blinker.png");
    state.ui.dot = Tachyon_CreateUIElement("./cosmodrone/assets/ui/dot.png");
    state.ui.target_indicator = Tachyon_CreateUIElement("./cosmodrone/assets/ui/target.png");
    state.ui.mini_target_indicator = Tachyon_CreateUIElement("./cosmodrone/assets/ui/mini-target.png");
    state.ui.target_focus = Tachyon_CreateUIElement("./cosmodrone/assets/ui/target-focus.png");
    state.ui.zone_target_indicator = Tachyon_CreateUIElement("./cosmodrone/assets/ui/zone-target.png");
    state.ui.selected_target_corner = Tachyon_CreateUIElement("./cosmodrone/assets/ui/selected-target-corner.png");
    state.ui.selected_target_center = Tachyon_CreateUIElement("./cosmodrone/assets/ui/selected-target-center.png");
    state.ui.flight_meter = Tachyon_CreateUIElement("./cosmodrone/assets/ui/flight-meter.png");
    state.ui.plane_meter = Tachyon_CreateUIElement("./cosmodrone/assets/ui/plane-meter.png");
    state.ui.meter_indicator_green = Tachyon_CreateUIElement("./cosmodrone/assets/ui/meter-indicator-green.png");
    state.ui.meter_indicator_red = Tachyon_CreateUIElement("./cosmodrone/assets/ui/meter-indicator-red.png");
    state.ui.meter_indicator_white = Tachyon_CreateUIElement("./cosmodrone/assets/ui/meter-indicator-white.png");

    state.ui.cascadia_mono_20 = Tachyon_CreateUIText("./fonts/CascadiaMonoNF.ttf", 20);
    state.ui.cascadia_mono_26 = Tachyon_CreateUIText("./fonts/CascadiaMonoNF.ttf", 26);
    state.ui.cascadia_mono_32 = Tachyon_CreateUIText("./fonts/CascadiaMonoNF.ttf", 32);
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
  objects(meshes.hud_flight_curve).disabled = state.is_editor_active;
  objects(meshes.drone_wireframe).disabled = state.is_editor_active;
  objects(meshes.antenna_3_wireframe).disabled = state.is_editor_active;

  // @todo dev mode only
  if (state.is_editor_active) {
    Editor::HandleEditor(tachyon, state, dt);

    return;
  } else {
    objects(state.meshes.editor_guideline).disabled = true;
  }

  HandleInputs(tachyon, state, dt);

  Autopilot::HandleAutopilot(tachyon, state, dt);
  Piloting::HandlePiloting(tachyon, state, dt);

  HandleCamera(tachyon, state, dt);
  HandleFlightGuides(tachyon, state, dt);
  HandleDrone(tachyon, state, dt);

  TargetSystem::HandleTargetTrackers(tachyon, state, dt);
  TargetSystem::UpdateTargetStats(tachyon, state);
  HUDSystem::HandleHUD(tachyon, state, dt);
  WorldBehavior::UpdateWorld(tachyon, state, dt);

  // @todo dev mode only
  if (tachyon->show_developer_tools) {
    UpdateShipDebugVectors(tachyon, state);
    ShowDevLabels(tachyon, state);
  }

  // @todo factor
  {
    auto s = Tachyon_GetMicroseconds();

    // 2-LoD
    // @todo process groups of meshes per frame
    Tachyon_UseLodByDistance(tachyon, meshes.girder_3, 80000.f);
    Tachyon_UseLodByDistance(tachyon, meshes.girder_5, 100000.f);
    Tachyon_UseLodByDistance(tachyon, meshes.girder_4_core, 200000.f);
    Tachyon_UseLodByDistance(tachyon, meshes.girder_6_frame, 150000.f);
    Tachyon_UseLodByDistance(tachyon, meshes.beam_1, 70000.f);

    Tachyon_UseLodByDistance(tachyon, meshes.mega_girder_1, 200000.f);

    Tachyon_UseLodByDistance(tachyon, meshes.machine_2, 70000.f);
    Tachyon_UseLodByDistance(tachyon, meshes.antenna_1, 70000.f);

    Tachyon_UseLodByDistance(tachyon, meshes.grate_1, 50000.f, 150000.f);
    Tachyon_UseLodByDistance(tachyon, meshes.grate_2, 100000.f);

    Tachyon_UseLodByDistance(tachyon, meshes.silo_2, 80000.f);

    Tachyon_UseLodByDistance(tachyon, meshes.silo_3_body, 80000.f);
    Tachyon_UseLodByDistance(tachyon, meshes.silo_3_frame, 80000.f);

    Tachyon_UseLodByDistance(tachyon, meshes.habitation_1_core, 80000.f);
    Tachyon_UseLodByDistance(tachyon, meshes.habitation_1_insulation, 80000.f);

    Tachyon_UseLodByDistance(tachyon, meshes.procedural_track_1, 300000.f);
    Tachyon_UseLodByDistance(tachyon, meshes.track_1_frame, 300000.f);

    // 3-LoD
    Tachyon_UseLodByDistance(tachyon, meshes.girder_1b, 100000.f, 200000.f);
    Tachyon_UseLodByDistance(tachyon, meshes.girder_2, 80000.f, 150000.f);
    Tachyon_UseLodByDistance(tachyon, meshes.machine_1, 70000.f, 150000.f);
    Tachyon_UseLodByDistance(tachyon, meshes.machine_3, 70000.f, 150000.f);

    // Use LoD 3 on the following meshes, which lack geometry for LoD 3.
    // This effectively distance-culls them at the second distance threshold,
    // since we do not render LoDs without geometry.
    Tachyon_UseLodByDistance(tachyon, meshes.girder_1, 120000.f, 180000.f);
    Tachyon_UseLodByDistance(tachyon, meshes.girder_4_frame, 70000.f, 200000.f);
    Tachyon_UseLodByDistance(tachyon, meshes.habitation_1_frame, 80000.f, 120000.f);
    Tachyon_UseLodByDistance(tachyon, meshes.habitation_3_frame, 40000.f, 120000.f);
    Tachyon_UseLodByDistance(tachyon, meshes.light_3_bulb, 20000.f, 100000.f);

    auto t = Tachyon_GetMicroseconds() - s;

    // @todo dev mode only
    add_dev_label("HandleLevelsOfDetail", std::to_string(t) + "us");
  }
}