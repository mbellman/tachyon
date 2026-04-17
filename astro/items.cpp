#include <map>

#include "astro/items.h"
#include "astro/collision_system.h"
#include "astro/sfx.h"
#include "astro/ui_system.h"

using namespace astro;

static std::map<std::string, ItemType> item_map = {
  { "magic_wand", MAGIC_WAND }
};

static void SpawnItemObject(Tachyon* tachyon, State& state, const tVec3f& position, ItemType item_type) {
  auto& meshes = state.meshes;

  switch (item_type) {
    // Wand
    case MAGIC_WAND: {
      auto& item = objects(meshes.player_wand)[0];

      item.position = position;
      item.position.y = CollisionSystem::QueryGroundHeight(state, position.x, position.z);
      item.position.y += 250.f;

      // @todo precalculate
      item.rotation = Quaternion::fromAxisAngle(tVec3f(0, 0, 1.f), -t_HALF_PI) *
      Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), 0.5f);

      item.scale = tVec3f(800.f);
      item.color = tVec3f(1.f, 0.6f, 0.2f);
      item.material = tVec4f(1.f, 0, 0, 0.4f);

      commit(item);
    }

    default:
      break;
  }
}

static std::string GetCollectItemDialogue(ItemType item_type) {
  switch (item_type) {
    case GATE_KEY:
      return "Collected the gate key.";
    case MAGIC_WAND:
      return "Retrieved the alchemist's wand.";
    default:
      return "Collected [unknown item].";
  }
}

ItemType Items::ItemNameToType(const std::string& item_name) {
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
    auto item_type = ItemNameToType(entity.item_pickup_name);

    if (item_type == ITEM_UNSPECIFIED) {
      add_console_message("Item '" + entity.item_pickup_name + "' doesn't map to a known item type", tVec3f(1.f, 0.9f, 0.4f));

      continue;
    }

    if (!Items::HasItem(state, item_type)) {
      SpawnItemObject(tachyon, state, entity.position, item_type);
    }
  }
}

void Items::CollectItem(Tachyon* tachyon, State& state, ItemType item_type) {
  Item item;
  item.type = item_type;

  state.inventory.push_back(item);

  auto dialogue = GetCollectItemDialogue(item_type);

  UISystem::ShowDialogue(tachyon, state, dialogue.c_str());
}


bool Items::HasItem(const State& state, ItemType item_type) {
  for (auto& item : state.inventory) {
    if (item.type == item_type) {
      return true;
    }
  }

  return false;
}