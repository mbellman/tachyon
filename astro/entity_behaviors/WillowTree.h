#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior WillowTree {
    addMeshes() {
      meshes.willow_tree_placeholder = CUBE_MESH(500);
      meshes.willow_tree_trunk = CUBE_MESH(500);
    }

    getMeshes() {
      return_meshes({
        meshes.willow_tree_trunk
      });
    }

    getPlaceholderMesh() {
      return meshes.willow_tree_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      // @todo @optimize only iterate over on-screen/in-range entities
      // once that list is built
      for_entities(state.willow_trees) {
        auto& tree = state.willow_trees[i];

        auto& trunk = objects(meshes.willow_tree_trunk)[i];

        trunk.position = tree.position;
        trunk.scale = tree.scale;
        trunk.rotation = tree.orientation;
        trunk.color = tree.tint;

        // Collision
        tree.visible_scale = trunk.scale;

        commit(trunk);
      }
    }
  };
}