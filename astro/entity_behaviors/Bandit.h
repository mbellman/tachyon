#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior Bandit {
    addMeshes() {
      meshes.bandit_placeholder = CUBE_MESH(500);
      meshes.bandit = CUBE_MESH(500);
    }

    getMeshes() {
      return_meshes({
        meshes.bandit
      });
    }

    getPlaceholderMesh() {
      return meshes.bandit_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      // @todo @optimize only iterate over on-screen/in-range entities
      // once that list is built
      for_entities(state.bandits) {
        auto& entity = state.bandits[i];
        auto& bandit = objects(meshes.bandit)[i];

        bandit.position = entity.position;
        bandit.scale = entity.scale;
        bandit.rotation = entity.orientation;
        bandit.color = entity.tint;

        // Collision
        entity.visible_scale = bandit.scale;

        commit(bandit);
      }
    }
  };
}