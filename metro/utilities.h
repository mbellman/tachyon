#pragma once

#include "metro/game_state.h"

namespace metro {
  int32 CreateUniqueId();
  tVec3f UnitBikeToWorldPosition(const Bicycle& bike, const tVec3f& position);
  tVec3f UnitVisualBikeToWorldPosition(const Bicycle& bike, const tVec3f& position);
  tVec3f UnitObjectToWorldPosition(const tObject object, const tVec3f& position);
  tVec3f GetMovementDirection(const Bicycle& bike);
  Bicycle* GetActiveBicycle(State& state);
}