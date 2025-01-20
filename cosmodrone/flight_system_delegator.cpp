#include "cosmodrone/drone_flight_system.h"
#include "cosmodrone/flight_system_delegator.h"

#define is_flying(mode) (state.flight_system == FlightSystem::mode)

using namespace Cosmodrone;

void FlightSystemDelegator::Forward(State& state, const float dt) {
  if (is_flying(DRONE)) {
    DroneFlightSystem::ControlledThrustForward(state, dt);
  }
}

void FlightSystemDelegator::Back(State& state, const float dt) {

}

void FlightSystemDelegator::Left(State& state, const float dt) {

}

void FlightSystemDelegator::Right(State& state, const float dt) {

}

void FlightSystemDelegator::RollLeft(State& state, const float dt) {

}

void FlightSystemDelegator::RollRight(State& state, const float dt) {

}

void FlightSystemDelegator::AutoStop(State& state, const float dt) {

}

void FlightSystemDelegator::AutoForward(State& state, const float dt) {

}

void FlightSystemDelegator::DockOrEject(State& state, const float dt) {

}