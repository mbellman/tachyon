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

    createPlaceholder() {
      auto& meshes = state.meshes;

      return create(meshes.shrub_placeholder);
    }

    destroyPlaceholders() {
      auto& meshes = state.meshes;

      remove_all(meshes.shrub_placeholder);
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