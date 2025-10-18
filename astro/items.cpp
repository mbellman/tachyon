#include <map>

#include "astro/items.h"

using namespace astro;

static std::map<std::string, ItemType> item_map = {
  { "astrolabe_bottom_left", ASTROLABE_BOTTOM_LEFT },
  { "astrolabe_bottom_right", ASTROLABE_BOTTOM_RIGHT },
  { "astrolabe_upper_right", ASTROLABE_UPPER_RIGHT },
};

ItemType Items::GetItemType(const std::string& item_name) {
  if (item_map.find(item_name) == item_map.end()) {
    return ITEM_UNSPECIFIED;
  }

  return item_map.at(item_name);
}