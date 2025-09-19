#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior SmallStoneBridge {
    spawned() {
      auto& meshes = state.meshes;

      create(meshes.small_stone_bridge_base);
      create(meshes.small_stone_bridge_columns);
    }

    destroyed() {
      auto& meshes = state.meshes;

      RemoveLastObject(tachyon, meshes.small_stone_bridge_base);
      RemoveLastObject(tachyon, meshes.small_stone_bridge_columns);
    }

    createPlaceholder() {
      auto& meshes = state.meshes;
      auto& placeholder = create(meshes.small_stone_bridge_placeholder);

      placeholder.position = entity.position;
      placeholder.scale = entity.scale;
      placeholder.rotation = entity.orientation;
      placeholder.color = entity.tint;

      commit(placeholder);

      return placeholder;
    }

    destroyPlaceholders() {
      auto& meshes = state.meshes;

      remove_all(meshes.small_stone_bridge_placeholder);
    }

    timeEvolve() {
      auto& meshes = state.meshes;
      const float lifetime = 60.f;

      for_entities(state.small_stone_bridges) {
        const auto& bridge = state.small_stone_bridges[i];
        auto& base = objects(meshes.small_stone_bridge_base)[i];
        auto& columns = objects(meshes.small_stone_bridge_columns)[i];
        // float life_progress = GetLivingEntityProgress(state, bridge, lifetime);

        base.position = bridge.position;
        base.scale = bridge.scale;
        base.rotation = bridge.orientation;
        base.color = bridge.tint;

        columns.position = bridge.position;
        columns.scale = bridge.scale;
        columns.rotation = bridge.orientation;
        columns.color = bridge.tint;

        commit(base);
        commit(columns);
      }
    }
  };
}