#include "cosmodrone/fighter_flight_system.h"

using namespace Cosmodrone;

const float ACCELERATION = 50000.f;

void FighterFlightSystem::ControlledThrustForward(State& state, const float dt) {
  state.ship_velocity += state.ship_rotation_basis.forward * ACCELERATION * dt;
  state.ship_rotate_to_target_speed += dt;
  state.flight_mode = FlightMode::MANUAL_CONTROL;
}