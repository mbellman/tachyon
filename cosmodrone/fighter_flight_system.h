#pragma once

#include "cosmodrone/game_types.h"

namespace Cosmodrone {
  namespace FighterFlightSystem {
    void ControlledThrustForward(State& state, const float dt);
    void RollLeft(State& state, const float dt);
    void RollRight(State& state, const float dt);
    void ChangePitch(State& state, const float dt, const float pitch_factor);
    void HandlePitch(State& state, const float dt);
  }
}