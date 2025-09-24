#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior RiverLog {
    spawned() {
      auto& meshes = state.meshes;

      create(meshes.river_log);
    }

    destroyed() {
      auto& meshes = state.meshes;

      RemoveLastObject(tachyon, meshes.river_log);
    }

    createPlaceholder() {
      auto& meshes = state.meshes;

      return create(meshes.river_log_placeholder);
    }

    destroyPlaceholders() {
      auto& meshes = state.meshes;

      remove_all(meshes.river_log_placeholder);
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      // @todo @optimize only iterate over on-screen/in-range entities
      // once that list is built
      for_entities(state.river_logs) {
        auto& entity = state.river_logs[i];
        auto& trunk = objects(meshes.river_log)[i];
        float oscillation = sinf(tachyon->running_time * 0.6f + entity.position.x);

        trunk.position = entity.position;
        trunk.position.y = state.water_level + 120.f * sinf(oscillation);
        trunk.rotation = entity.orientation;
        trunk.color = entity.tint;
        trunk.scale = entity.scale;

        // Collision
        entity.visible_scale = trunk.scale;

        commit(trunk);
      }
    }
  };
}