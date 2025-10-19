#include <map>

#include "astro/items.h"
#include "astro/sfx.h"
#include "astro/ui_system.h"

using namespace astro;

static std::map<std::string, ItemType> item_map = {
  { "astrolabe_lower_left", ASTROLABE_LOWER_LEFT },
  { "astrolabe_lower_right", ASTROLABE_LOWER_RIGHT },
  { "astrolabe_upper_right", ASTROLABE_UPPER_RIGHT },
};

static void SpawnItemObject(Tachyon* tachyon, State& state, const tVec3f& position, ItemType item_type) {
  auto& meshes = state.meshes;

  switch (item_type) {
    // Astrolabe parts
    case ASTROLABE_LOWER_LEFT:
    caseASTROLABE_LOWER_RIGHT:
    case ASTROLABE_UPPER_RIGHT: {
      auto& item = create(meshes.item_astro_part);

      item.position = position;
      item.scale = tVec3f(500.f);
      item.color = tVec3f(0, 0, 1.f);

      commit(item);

      break;
    }

    default:
      break;
  }
}

ItemType Items::GetItemType(const std::string& item_name) {
  if (item_map.find(item_name) == item_map.end()) {
    return ITEM_UNSPECIFIED;
  }

  return item_map.at(item_name);
}

void Items::SpawnItemObjects(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  remove_all(meshes.item_astro_part);

  for_entities(state.item_pickups) {
    auto& entity = state.item_pickups[i];
    auto item_type = GetItemType(entity.item_pickup_name);

    if (item_type == ITEM_UNSPECIFIED) {
      add_console_message("Unknown item name: " + entity.item_pickup_name, tVec3f(1.f, 0.9f, 0.4f));

      continue;
    }

    SpawnItemObject(tachyon, state, entity.position, item_type);
  }
}

void Items::HandleItemPickup(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;
  tVec3f& player_position = state.player_position;

  // @todo iterate over item_pickup entities instead
  for (auto& part : objects(meshes.item_astro_part)) {
    if (tVec3f::distance(player_position, part.position) < 700.f) {
      // @todo per-item messaging
      UISystem::ShowDialogue(tachyon, state, "Acquired lower-left astrolabe fragment.");
      // @temporary
      Sfx::PlaySound(SFX_SPELL_STUN);

      remove(part);

      // @temporary
      Item item;
      item.type = ASTROLABE_LOWER_LEFT;

      state.inventory.push_back(item);

      break;
    }
  }
}

bool Items::HasItem(const State& state, ItemType item_type) {
  for (auto& item : state.inventory) {
    if (item.type == item_type) {
      return true;
    }
  }

  return false;
}