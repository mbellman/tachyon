#pragma once

#include "metro/game_state.h"

namespace metro {
  namespace Serialization {
    std::string EntityTypeToString(EntityType type);
    void SaveWorldData(const State& state);
  }
}