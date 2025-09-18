#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior OakTree {
    spawned() {
      auto& meshes = state.meshes;

      create(meshes.oak_tree_trunk);
    }

    destroyed() {
      auto& meshes = state.meshes;

      RemoveLastObject(tachyon, meshes.oak_tree_trunk);
    }

    createPlaceholder() {
      auto& meshes = state.meshes;
      auto& placeholder = create(meshes.oak_tree_placeholder);

      placeholder.position = entity.position;
      placeholder.scale = tVec3f(500.f, 2000.f, 500.f);
      placeholder.rotation = entity.orientation;
      placeholder.color = tVec3f(1.f, 0.6f, 0.3f);

      commit(placeholder);

      return placeholder;
    }

    destroyPlaceholders() {
      auto& meshes = state.meshes;

      remove_all(meshes.oak_tree_placeholder);
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      const float lifetime = 200.f;

      // @todo @optimize only iterate over on-screen/in-range entities
      // once that list is built
      for_entities(state.oak_trees) {
        const auto& tree = state.oak_trees[i];
        float life_progress = GetLivingEntityProgress(state, tree, lifetime);

        // @todo factor
        auto& trunk = objects(meshes.oak_tree_trunk)[i];
        float trunk_height = 1.f - powf(1.f - life_progress, 4.f);
        float trunk_thickness = -(cosf(t_PI * life_progress) - 1.f) / 2.f;

        trunk.scale = tree.scale * tVec3f(
          0.02f + 0.18f * trunk_thickness,
          trunk_height,
          0.02f + 0.18f * trunk_thickness
        );

        trunk.position = tree.position;
        trunk.position.y = tree.position.y - tree.scale.y * (1.f - trunk_height);

        trunk.rotation = tree.orientation;
        trunk.color = tree.tint;

        commit(trunk);
      }
    }
  };
}