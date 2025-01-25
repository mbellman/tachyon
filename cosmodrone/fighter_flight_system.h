#pragma once

#include "cosmodrone/game_types.h"

namespace Cosmodrone {
  namespace FighterFlightSystem {
    void ControlledThrustForward(State& state, const float dt);
    void RollLeft(State& state, const float dt);
    void RollRight(State& state, const float dt);
  }
}