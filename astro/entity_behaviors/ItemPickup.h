#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior ItemPickup {
    addMeshes() {
      meshes.item_pickup_placeholder = SPHERE_MESH(500);

      mesh(meshes.item_pickup_placeholder).shadow_cascade_ceiling = 0;
    }

    getMeshes() {
      // Item pickups don't have a specific in-game object.
      // Instead, their type influences which object appears
      // in their spot; that object is generated elsewhere.
      return_meshes({});
    }

    getPlaceholderMesh() {
      return meshes.item_pickup_placeholder;
    }

    timeEvolve() {
      // Do nothing
    }
  };
}