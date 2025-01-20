#pragma once

#include "cosmodrone/game_types.h"

namespace Cosmodrone {
  namespace FlightSystemDelegator {
    void Forward(State& state, const float dt);
    void PullBack(State& state, const float dt, const float factor);
    void Left(State& state, const float dt);
    void Right(State& state, const float dt);
    void RollLeft(State& state, const float dt);
    void RollRight(State& state, const float dt);
    void AutoPrograde(State& state, const float dt);
    void AutoStop(State& state, const float dt);
    void DockOrUndock(Tachyon* tachyon, State& state, const float dt);
  }
}