#pragma once

#include "metro/game_state.h"

namespace metro {
  namespace Serialization {
    void SaveWorldData(const State& state, const std::string& world_name);
  }
}