#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior WillowTree {
    spawned() {
      auto& meshes = state.meshes;

      create(meshes.willow_tree_trunk);
    }

    destroyed() {
      auto& meshes = state.meshes;

      RemoveLastObject(tachyon, meshes.willow_tree_trunk);
    }

    createPlaceholder() {
      auto& meshes = state.meshes;
      auto& placeholder = create(meshes.willow_tree_placeholder);

      placeholder.position = entity.position;
      placeholder.scale = entity.scale;
      placeholder.rotation = entity.orientation;
      placeholder.color = entity.tint;

      commit(placeholder);

      return placeholder;
    }

    destroyPlaceholders() {
      auto& meshes = state.meshes;

      remove_all(meshes.shrub_placeholder);
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      // @todo @optimize only iterate over on-screen/in-range entities
      // once that list is built
      for_entities(state.willow_trees) {
        const auto& tree = state.willow_trees[i];

        auto& trunk = objects(meshes.willow_tree_trunk)[i];

        trunk.position = tree.position;
        trunk.scale = tree.scale;
        trunk.rotation = tree.orientation;
        trunk.color = tree.tint;

        commit(trunk);
      }
    }
  };
}