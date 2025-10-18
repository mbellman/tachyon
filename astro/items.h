# pragma once

#include <string>

namespace astro {
  enum ItemType {
    ITEM_UNSPECIFIED = -1,
    ASTROLABE_BOTTOM_LEFT,
    ASTROLABE_BOTTOM_RIGHT,
    ASTROLABE_UPPER_RIGHT
  };

  struct Item {

  };

  namespace Items {
    ItemType GetItemType(const std::string& item_name);
  }
}