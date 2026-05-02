
#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior CastleTower {
    addMeshes() {
      meshes.castle_tower_placeholder = CUBE_MESH(500);
      meshes.castle_tower = CUBE_MESH(500);
    }

    getMeshes() {
      return_meshes({
        meshes.castle_tower
      });
    }

    getPlaceholderMesh() {
      return meshes.castle_tower_placeholder;
    }

    timeEvolve() {
      // @todo
    }
  };
}
  