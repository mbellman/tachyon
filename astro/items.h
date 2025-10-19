# pragma once

#include <string>

#include "astro/game_state.h"

namespace astro {
  namespace Items {
    ItemType GetItemType(const std::string& item_name);
    void SpawnItemObjects(Tachyon* tachyon, State& state);
    void HandleItemPickup(Tachyon* tachyon, State& state);
  }
}