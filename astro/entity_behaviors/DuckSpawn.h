
#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior DuckSpawn {
    addMeshes() {
      meshes.duck_spawn_placeholder = SPHERE_MESH(100);
    }

    getMeshes() {
      // Duck spawns don't have a specific in-game object
      return_meshes({});
    }

    getPlaceholderMesh() {
      return meshes.duck_spawn_placeholder;
    }

    timeEvolve() {
    }
  };
}
