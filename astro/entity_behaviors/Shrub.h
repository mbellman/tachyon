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
      auto& placeholder = create(meshes.shrub_placeholder);

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
      const float lifetime = 60.f;

      for_entities(state.shrubs) {
        const auto& shrub = state.shrubs[i];
        float life_progress = GetLivingEntityProgress(state, shrub, lifetime);

        // @todo factor
        auto& branches = objects(meshes.shrub_branches)[i];

        branches.position = shrub.position;
        branches.position.y = -1500.f + branches.scale.y;

        branches.scale = shrub.scale * sinf(life_progress * 0.75f * t_PI);

        branches.rotation = shrub.orientation;
        branches.color = shrub.tint;

        commit(branches);
      }
    }
  };
}