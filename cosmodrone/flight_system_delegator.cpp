#include "cosmodrone/autopilot.h"
#include "cosmodrone/drone_flight_system.h"
#include "cosmodrone/fighter_flight_system.h"
#include "cosmodrone/flight_system_delegator.h"

#define is_flying(mode) (state.flight_system == FlightSystem::mode)

using namespace Cosmodrone;

void FlightSystemDelegator::Forward(State& state, const float dt) {
  if (state.flight_system == FlightSystem::DRONE) {
    DroneFlightSystem::ControlledThrustForward(state, dt);
  }

  if (state.flight_system == FlightSystem::FIGHTER) {
    FighterFlightSystem::ControlledThrustForward(state, dt);
  }
}

void FlightSystemDelegator::PullBack(State& state, const float dt, const float factor) {
  if (state.flight_system == FlightSystem::DRONE) {
    DroneFlightSystem::ChangePitch(state, dt, factor);
  }

  if (state.flight_system == FlightSystem::FIGHTER) {
    // @todo
  }
}

void FlightSystemDelegator::Left(State& state, const float dt) {
  if (state.flight_system == FlightSystem::DRONE) {
    DroneFlightSystem::YawLeft(state, dt);
  }

  if (state.flight_system == FlightSystem::FIGHTER) {
    // @todo
  }
}

void FlightSystemDelegator::Right(State& state, const float dt) {
  if (state.flight_system == FlightSystem::DRONE) {
    DroneFlightSystem::YawRight(state, dt);
  }

  if (state.flight_system == FlightSystem::FIGHTER) {
    // @todo
  }
}

void FlightSystemDelegator::RollLeft(State& state, const float dt) {
  if (state.flight_system == FlightSystem::DRONE) {
    DroneFlightSystem::RollLeft(state, dt);
  }

  if (state.flight_system == FlightSystem::FIGHTER) {
    // @todo
  }
}

void FlightSystemDelegator::RollRight(State& state, const float dt) {
  if (state.flight_system == FlightSystem::DRONE) {
    DroneFlightSystem::RollRight(state, dt);
  }

  if (state.flight_system == FlightSystem::FIGHTER) {
    // @todo
  }
}

void FlightSystemDelegator::AutoPrograde(State& state, const float dt) {
  // @todo create a method in autopilot.cpp
  if (state.flight_system == FlightSystem::DRONE) {
    state.flight_mode = FlightMode::AUTO_PROGRADE;
    state.ship_rotate_to_target_speed = 0.f;
  }
}

void FlightSystemDelegator::AutoStop(State& state, const float dt) {
  // @todo create a method in autopilot.cpp
  if (state.flight_system == FlightSystem::DRONE) {
    state.flight_mode = FlightMode::AUTO_RETROGRADE;
    // @todo define the retrograde direction correctly (as the anti-vector of velocity).
    // We're doing this for now because of quirks with the player drone model, which should
    // probably be correctly oriented.
    state.retrograde_direction = state.ship_velocity_basis.forward;
    state.ship_rotate_to_target_speed = 0.f;
  }
}

void FlightSystemDelegator::DockOrUndock(Tachyon* tachyon, State& state, const float dt) {
  if (Autopilot::IsDocked(state)) {
    Autopilot::Undock(tachyon, state);
  } else {
    Autopilot::AttemptDockingProcedure(state);
  }
}