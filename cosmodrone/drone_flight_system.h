#pragma once

#include "cosmodrone/game_types.h"

namespace Cosmodrone {
  namespace DroneFlightSystem {
    void ThrustForward(State& state, const float dt, const float rate);
    void ControlledThrustForward(State& state, const float dt);
    void RollLeft(State& state, const float dt);
    void RollRight(State& state, const float dt);
    void YawLeft(State& state, const float dt);
    void YawRight(State& state, const float dt);
    void ChangePitch(State& state, const float dt, const float pitch);
    void HandlePitch(State& state, const float dt);
  }
}