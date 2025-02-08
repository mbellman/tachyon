#include "cosmodrone/drone_flight_system.h"

using namespace Cosmodrone;

const static float ACCELERATION = 2000.f;

void DroneFlightSystem::ThrustForward(State& state, const float dt, const float rate) {
  state.ship_velocity += state.ship_rotation_basis.forward * rate * dt;

  state.jets_intensity += 3.f * dt;
}

void DroneFlightSystem::ControlledThrustForward(State& state, const float dt) {
  float speed = state.ship_velocity.magnitude();

  // Go faster when starting acceleration
  float fast_start_ratio = speed / 10000.f;
  if (fast_start_ratio > 1.f) fast_start_ratio = 1.f;
  fast_start_ratio = 1.f - fast_start_ratio;

  float proper_acceleration = ACCELERATION + 2.f * fast_start_ratio * ACCELERATION;

  // Slow down along existing the velocity vector,
  // proportional to directional change
  {
    float forward_alignment = tVec3f::dot(state.ship_rotation_basis.forward, state.ship_velocity_basis.forward);
    if (forward_alignment < 0.f) forward_alignment = 0.f;

    float slowdown_factor = 3.f * (1.f - forward_alignment);

    state.ship_velocity -= state.ship_velocity_basis.forward * slowdown_factor * proper_acceleration * dt;
  }

  float view_alignment = std::max(0.f, tVec3f::dot(state.ship_rotation_basis.forward, state.view_forward_direction));

  // Accelerate along the ship forward direction
  ThrustForward(state, dt, proper_acceleration * powf(view_alignment, 3.f));

  // Bias the drone trajectory toward its forward direction
  // as it aligns with the view forward. This makes it easier
  // to aim without needing to counteract drift.
  {
    const float bias = 0.0035f;
    float updated_speed = state.ship_velocity.magnitude();

    state.ship_velocity = (
      state.ship_velocity.unit() +
      state.ship_rotation_basis.forward * view_alignment * bias
    ).unit() * updated_speed;
  }

  state.controlled_thrust_duration += dt;
  state.ship_rotate_to_target_speed += 5.f * (1.f - state.ship_pitch_factor) * dt;
  state.flight_mode = FlightMode::MANUAL_CONTROL;
}

// @todo combine Roll functions
void DroneFlightSystem::RollLeft(State& state, const float dt) {
  state.camera_roll_speed += dt;
  state.ship_rotate_to_target_speed += 5.f * dt;
  state.flight_mode = FlightMode::MANUAL_CONTROL;
}

void DroneFlightSystem::RollRight(State& state, const float dt) {
  state.camera_roll_speed -= dt;
  state.ship_rotate_to_target_speed += 5.f * dt;
  state.flight_mode = FlightMode::MANUAL_CONTROL;
}

// @todo combine Yaw functions
void DroneFlightSystem::YawLeft(State& state, const float dt) {
  state.ship_rotate_to_target_speed += 5.f * dt;
  state.camera_yaw_speed += 5.f * dt;
  state.flight_mode = FlightMode::MANUAL_CONTROL;
  state.flight_target_reticle_offset.x += 0.7f * state.camera_yaw_speed * dt;
}

// @todo combine Yaw functions
void DroneFlightSystem::YawRight(State& state, const float dt) {
  state.ship_rotate_to_target_speed += 5.f * dt;
  state.camera_yaw_speed += 5.f * dt;
  state.flight_mode = FlightMode::MANUAL_CONTROL;
  state.flight_target_reticle_offset.x -= 0.7f * state.camera_yaw_speed * dt;
}

void DroneFlightSystem::ChangePitch(State& state, const float dt, const float pitch_factor) {
  state.ship_pitch_factor += pitch_factor * dt;
  if (state.ship_pitch_factor < -1.f) state.ship_pitch_factor = -1.f;
  if (state.ship_pitch_factor > 1.f) state.ship_pitch_factor = 1.f;

  state.flight_mode = FlightMode::MANUAL_CONTROL;
  state.flight_target_reticle_offset.y += 0.4f * state.ship_pitch_factor * dt;
}

void DroneFlightSystem::HandlePitch(State& state, const float dt) {
  float ship_speed = state.ship_velocity.magnitude();
  float slowdown_factor = 5.f * state.ship_pitch_factor * ACCELERATION;
  float acceleration_factor = 20.f * state.ship_pitch_factor * ACCELERATION;

  state.ship_velocity -= state.ship_velocity_basis.forward * slowdown_factor * dt;

  ThrustForward(state, dt, acceleration_factor);

  state.jets_intensity -= 3.f * dt;
  state.jets_intensity += 10.f * std::abs(state.ship_pitch_factor) * dt;

  state.ship_velocity = state.ship_velocity.unit() * ship_speed;

  state.target_ship_rotation =
    state.target_ship_rotation *
    Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), 0.3f * state.ship_pitch_factor);

  state.ship_rotate_to_target_speed += 2.f * abs(state.ship_pitch_factor) * dt;
}
