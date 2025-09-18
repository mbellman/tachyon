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

    placeholderSpawned() {
      auto& meshes = state.meshes;
      auto& placeholder = create(meshes.willow_tree_placeholder);

      placeholder.position = entity.position;
      placeholder.scale = tVec3f(500.f, 2000.f, 500.f);
      placeholder.rotation = entity.orientation;
      placeholder.color = tVec3f(1.f, 0.6f, 0.3f);

      commit(placeholder);

      return placeholder;
    }

    placeholdersDestroyed() {
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