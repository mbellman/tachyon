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
  state.camera_roll_speed += 4.f * dt;
  state.flight_mode = FlightMode::MANUAL_CONTROL;
}

void FighterFlightSystem::RollRight(State& state, const float dt) {
  state.camera_roll_speed -= 4.f * dt;
  state.flight_mode = FlightMode::MANUAL_CONTROL;
}

void FighterFlightSystem::ChangePitch(State& state, const float dt, const float pitch_factor) {
  state.ship_pitch_factor += pitch_factor * dt;
  if (state.ship_pitch_factor < -1.f) state.ship_pitch_factor = -1.f;
  if (state.ship_pitch_factor > 1.f) state.ship_pitch_factor = 1.f;

  state.flight_mode = FlightMode::MANUAL_CONTROL;
  state.flight_target_reticle_offset.y += 0.4f * state.ship_pitch_factor * dt;
}

void FighterFlightSystem::HandlePitch(State& state, const float dt) {
  float ship_speed = state.ship_velocity.magnitude();
  float slowdown_factor = 5.f * state.ship_pitch_factor * ACCELERATION;
  float acceleration_factor = 20.f * state.ship_pitch_factor * ACCELERATION;
  // @todo use external constant for max speed
  float rotation_blend = 0.75f * (ship_speed / 50000.f) * state.ship_pitch_factor;

  state.ship_velocity -= state.ship_velocity_basis.forward * slowdown_factor * dt;
  state.ship_velocity += state.view_forward_direction * ACCELERATION * dt;
  state.ship_velocity = state.ship_velocity.unit() * ship_speed;

  state.jets_intensity -= 3.f * dt;
  state.jets_intensity += 10.f * std::abs(state.ship_pitch_factor) * dt;

  state.target_ship_rotation =
    state.target_ship_rotation *
    Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), rotation_blend);

  state.ship_rotate_to_target_speed += 5.f * abs(state.ship_pitch_factor) * dt;
}