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

    placeholderSpawned() {
      auto& meshes = state.meshes;
      auto& placeholder = create(meshes.oak_tree_placeholder);

      placeholder.position = entity.position;
      placeholder.scale = tVec3f(500.f, 2000.f, 500.f);
      placeholder.color = tVec3f(1.f, 0.6f, 0.3f);

      commit(placeholder);

      return placeholder;
    }

    placeholdersDestroyed() {
      auto& meshes = state.meshes;

      remove_all(meshes.oak_tree_placeholder);
    }
  };
}