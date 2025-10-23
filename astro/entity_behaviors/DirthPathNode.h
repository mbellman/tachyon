#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior DirtPathNode {
    addMeshes() {
      meshes.dirt_path_node_placeholder = SPHERE_MESH(500);
    }

    getMeshes() {
      // Path nodes don't have a specific in-game object;
      // path segments are generated between them.
      return_meshes({});
    }

    getPlaceholderMesh() {
      return meshes.dirt_path_node_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      for_entities(state.dirt_path_nodes) {
        auto& entity = state.dirt_path_nodes[i];

        // @todo
      }
    }
  };
}