#pragma once

#include "cosmodrone/game_types.h"

namespace Cosmodrone {
  namespace FlightSystem {
    void ThrustForward(State& state, const float dt, const float rate);
    void ControlledThrustForward(State& state, const float dt);
    void RollLeft(State& state, const float dt);
    void RollRight(State& state, const float dt);
    void YawLeft(State& state, const float dt);
    void YawRight(State& state, const float dt);
    void PullUpward(State& state, const float dt);
    void PitchDownward(State& state, const float dt);
  }
}