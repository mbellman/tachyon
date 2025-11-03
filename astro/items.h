# pragma once

#include <string>

#include "astro/game_state.h"

namespace astro {
  namespace Items {
    ItemType ItemNameToType(const std::string& item_name);
    void SpawnItemObjects(Tachyon* tachyon, State& state);
    void HandleItemPickup(Tachyon* tachyon, State& state);
    void CollectItem(Tachyon* tachyon, State& state, ItemType item_type);
    bool HasItem(const State& state, ItemType item_type);
  }
}