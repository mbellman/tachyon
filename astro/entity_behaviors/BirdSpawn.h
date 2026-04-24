#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior BirdSpawn {
    addMeshes() {
      meshes.bird_spawn_placeholder = SPHERE_MESH(100);

      mesh(meshes.bird_spawn_placeholder).shadow_cascade_ceiling = 0;
    }

    getMeshes() {
      // Bird spawns don't have a specific in-game object
      return_meshes({});
    }

    getPlaceholderMesh() {
      return meshes.bird_spawn_placeholder;
    }

    timeEvolve() {
      // @see dynamic_fauna.cpp -> HandleTinyBirds()
    }
  };
}