#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior StonePathNode {
    addMeshes() {
      meshes.stone_path_node_placeholder = SPHERE_MESH(500);

      mesh(meshes.stone_path_node_placeholder).shadow_cascade_ceiling = 0;
    }

    getMeshes() {
      // Path nodes don't have a specific in-game object;
      // path segments are generated between them.
      return_meshes({});
    }

    getPlaceholderMesh() {
      return meshes.stone_path_node_placeholder;
    }

    timeEvolve() {}
  };
}