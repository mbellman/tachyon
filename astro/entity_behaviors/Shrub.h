#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior Shrub {
    addMeshes() {
      meshes.shrub_placeholder = CUBE_MESH(500);
      meshes.shrub_branches = CUBE_MESH(500);
    }

    getMeshes() {
      return_meshes({
        meshes.shrub_branches
      });
    }

    getPlaceholderMesh() {
      return meshes.shrub_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;
      const float lifetime = 100.f;

      for_entities(state.shrubs) {
        auto& shrub = state.shrubs[i];
        float life_progress = GetLivingEntityProgress(state, shrub, lifetime);

        // @todo factor
        auto& branches = objects(meshes.shrub_branches)[i];

        branches.scale = shrub.scale * sinf(life_progress * t_PI);
        
        branches.position = shrub.position;
        branches.position.y -= (shrub.scale.y - branches.scale.y);

        branches.rotation = shrub.orientation;
        branches.color = shrub.tint;

        // Collision
        shrub.visible_scale = branches.scale;

        commit(branches);
      }
    }
  };
}