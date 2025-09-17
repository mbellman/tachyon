#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior Shrub {
    spawned() {
      auto& meshes = state.meshes;

      create(meshes.shrub_branches);
    }

    destroyed() {
      auto& meshes = state.meshes;

      RemoveLastObject(tachyon, meshes.shrub_branches);
    }

    placeholderSpawned() {
      auto& meshes = state.meshes;
      auto& placeholder = create(meshes.shrub_placeholder);

      placeholder.position = entity.position;
      placeholder.scale = tVec3f(800.f);
      placeholder.color = tVec3f(0.2f, 0.8f, 0.5f);

      commit(placeholder);

      return placeholder;
    }

    placeholdersDestroyed() {
      auto& meshes = state.meshes;

      remove_all(meshes.shrub_placeholder);
    }
  };
}