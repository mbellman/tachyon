#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior SwanSpawn {
    addMeshes() {
      meshes.swan_spawn_placeholder = SPHERE_MESH(500);

    }

    getMeshes() {
      return_meshes({});
    }

    getPlaceholderMesh() {
      // Swan spawns don't have a specific in-game object
      return meshes.swan_spawn_placeholder;
    }

    timeEvolve() {
      // @todo
    }
  };
}
