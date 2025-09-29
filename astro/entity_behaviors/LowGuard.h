#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior LowGuard {
    addMeshes() {
      meshes.low_guard_placeholder = CUBE_MESH(500);
      meshes.low_guard = CUBE_MESH(500);
    }

    getMeshes() {
      return_meshes({
        meshes.low_guard
      });
    }

    getPlaceholderMesh() {
      return meshes.low_guard_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      // @todo @optimize only iterate over on-screen/in-range entities
      // once that list is built
      for_entities(state.low_guards) {
        auto& entity = state.low_guards[i];
        auto& guard = objects(meshes.low_guard)[i];

        guard.position = entity.position;
        guard.scale = entity.scale;
        guard.rotation = entity.orientation;
        guard.color = entity.tint;

        // Collision
        entity.visible_scale = guard.scale;

        commit(guard);
      }
    }
  };
}