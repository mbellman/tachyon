#include "cosmodrone/flight_system.h"

using namespace Cosmodrone;

const static float ACCELERATION = 2000.f;

void FlightSystem::ThrustForward(State& state, const float dt, const float rate) {
  state.ship_velocity += state.ship_rotation_basis.forward * rate * dt;
}

void FlightSystem::ControlledThrustForward(State& state, const float dt) {
  float speed = state.ship_velocity.magnitude();
  float fast_start_ratio = speed / 4000.f;
  if (fast_start_ratio > 1.f) fast_start_ratio = 1.f;
  fast_start_ratio = 1.f - fast_start_ratio;

  float proper_acceleration = ACCELERATION + fast_start_ratio * ACCELERATION;

  // Slow down along existing the velocity vector,
  // proportional to directional change
  float forward_alignment = tVec3f::dot(state.ship_rotation_basis.forward, state.ship_velocity_basis.forward);
  float slowdown_factor = 2.f * (1.f - std::max(0.f, forward_alignment));

  state.ship_velocity -= state.ship_velocity_basis.forward * slowdown_factor * proper_acceleration * dt;

  // Accelerate along the ship forward direction
  ThrustForward(state, dt, proper_acceleration);

  state.ship_rotate_to_target_speed += 5.f * dt;
  state.flight_mode = FlightMode::MANUAL_CONTROL;
}

void FlightSystem::RollLeft(State& state, const float dt) {
  state.camera_roll_speed += dt;
  state.ship_rotate_to_target_speed += 5.f * dt;
  state.flight_mode = FlightMode::MANUAL_CONTROL;
}

void FlightSystem::RollRight(State& state, const float dt) {
  state.camera_roll_speed -= dt;
  state.ship_rotate_to_target_speed += 5.f * dt;
  state.flight_mode = FlightMode::MANUAL_CONTROL;
}

void FlightSystem::YawLeft(State& state, const float dt) {
  state.ship_rotate_to_target_speed += 5.f * dt;
  state.flight_mode = FlightMode::MANUAL_CONTROL;
}

void FlightSystem::YawRight(State& state, const float dt) {
  state.ship_rotate_to_target_speed += 5.f * dt;
  state.flight_mode = FlightMode::MANUAL_CONTROL;
}

void FlightSystem::PullUpward(State& state, const float dt) {
  float speed = state.ship_velocity.magnitude();

  state.ship_velocity -= state.ship_velocity_basis.forward * 5.f * ACCELERATION * dt;

  ThrustForward(state, dt, 10.f * ACCELERATION);

  state.ship_velocity = state.ship_velocity.unit() * speed;
  state.ship_rotate_to_target_speed += 5.f * dt;
  state.flight_mode = FlightMode::MANUAL_CONTROL;
}

void FlightSystem::PitchDownward(State& state, const float dt) {

}
