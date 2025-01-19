#pragma once

#include "cosmodrone/game_types.h"

namespace Cosmodrone {
  namespace FlightSystemDelegator {
    void Forward(State& state, const float dt);
    void Back(State& state, const float dt);
    void Left(State& state, const float dt);
    void Right(State& state, const float dt);
    void RollLeft(State& state, const float dt);
    void RollRight(State& state, const float dt);
    void AutoStop(State& state, const float dt);
    void AutoForward(State& state, const float dt);
    void DockOrEject(State& state, const float dt);
  }
}