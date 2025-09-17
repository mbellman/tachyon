#pragma once

#include "astro/entity_descriptions/description.h"

namespace astro {
  description OakTree {
    created() {
      auto& meshes = state.meshes;

      create(meshes.oak_tree_trunk);
    }

    destroyed() {
      auto& meshes = state.meshes;

      RemoveLastObject(tachyon, meshes.oak_tree_trunk);
    }

    placeholderCreated() {
      auto& placeholder = create(state.meshes.oak_tree_placeholder);

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