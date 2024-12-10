#include "cosmodrone/flight_system.h"

using namespace Cosmodrone;

const static float ACCELERATION = 2000.f;

void FlightSystem::ThrustForward(State& state, const float dt, const float rate) {
  state.ship_velocity += state.ship_rotation_basis.forward * rate * dt;
}

void FlightSystem::ControlledThrustForward(State& state, const float dt) {
  state.ship_velocity -= state.ship_velocity_basis.forward * (ACCELERATION * 0.7f) * dt;

  ThrustForward(state, dt, ACCELERATION);

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

}

void FlightSystem::PitchDownward(State& state, const float dt) {

}
