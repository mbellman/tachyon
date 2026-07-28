#pragma once

#include "metro/game_state.h"

namespace metro {
  tVec3f UnitBikeToWorldPosition(const Bicycle& bike, const tVec3f& position);
  tVec3f UnitVisualBikeToWorldPosition(const Bicycle& bike, const tVec3f& position);
  tVec3f UnitObjectToWorldPosition(const tObject object, const tVec3f& position);
  tVec3f GetTrueFacingDirection(const Bicycle& bike);
  Bicycle* GetActiveBicycle(State& state);
}