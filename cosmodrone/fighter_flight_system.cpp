#include "cosmodrone/fighter_flight_system.h"

using namespace Cosmodrone;

const float ACCELERATION = 500000.f;

void FighterFlightSystem::ControlledThrustForward(State& state, const float dt) {
  if (state.controlled_thrust_duration > 1.f) {
    state.ship_velocity += state.ship_rotation_basis.forward * ACCELERATION * dt;
  }

  state.ship_rotate_to_target_speed += dt;
  state.controlled_thrust_duration += dt;
  state.flight_mode = FlightMode::MANUAL_CONTROL;
}

void FighterFlightSystem::RollLeft(State& state, const float dt) {
  state.camera_roll_speed += dt;
  state.flight_mode = FlightMode::MANUAL_CONTROL;
}

void FighterFlightSystem::RollRight(State& state, const float dt) {
  state.camera_roll_speed -= dt;
  state.flight_mode = FlightMode::MANUAL_CONTROL;
}