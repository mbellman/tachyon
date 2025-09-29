#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior OakTree {
    addMeshes() {
      meshes.oak_tree_placeholder = CUBE_MESH(500);
      meshes.oak_tree_trunk = CUBE_MESH(500);
    }

    getMeshes() {
      return_meshes({
        meshes.oak_tree_trunk
      });
    }

    getPlaceholderMesh() {
      return meshes.oak_tree_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      const float lifetime = 200.f;

      // @todo @optimize only iterate over on-screen/in-range entities
      // once that list is built
      for_entities(state.oak_trees) {
        auto& tree = state.oak_trees[i];
        float life_progress = GetLivingEntityProgress(state, tree, lifetime);

        // @todo factor
        auto& trunk = objects(meshes.oak_tree_trunk)[i];
        float trunk_height = 1.f - powf(1.f - life_progress, 4.f);
        float trunk_thickness = 0.1f + 0.9f * -(cosf(t_PI * life_progress) - 1.f) / 2.f;

        trunk.scale = tree.scale * tVec3f(
          trunk_thickness,
          trunk_height,
          trunk_thickness
        );

        trunk.position = tree.position;
        trunk.position.y = tree.position.y - tree.scale.y * (1.f - trunk_height);

        trunk.rotation = tree.orientation;
        trunk.color = tree.tint;

        // Collision
        tree.visible_scale = trunk.scale;

        commit(trunk);
      }
    }
  };
}