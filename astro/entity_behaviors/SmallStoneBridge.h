#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior SmallStoneBridge {
    addMeshes() {
      meshes.small_stone_bridge_placeholder = MODEL_MESH("./astro/3d_models/small_stone_bridge/placeholder.obj", 500);
      meshes.small_stone_bridge_base = MODEL_MESH("./astro/3d_models/small_stone_bridge/base.obj", 500);
      meshes.small_stone_bridge_columns = MODEL_MESH("./astro/3d_models/small_stone_bridge/columns.obj", 500);

      mesh(meshes.small_stone_bridge_base).shadow_cascade_ceiling = 2;
      mesh(meshes.small_stone_bridge_columns).shadow_cascade_ceiling = 2;
    }

    getMeshes() {
      return_meshes({
        meshes.small_stone_bridge_base,
        meshes.small_stone_bridge_columns
      });
    }

    getPlaceholderMesh() {
      return meshes.small_stone_bridge_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      reset_instances(meshes.small_stone_bridge_base);
      reset_instances(meshes.small_stone_bridge_columns);

      for_entities(state.small_stone_bridges) {
        auto& entity = state.small_stone_bridges[i];

        if (abs(state.player_position.x - entity.position.x) > 25000.f) continue;
        if (abs(state.player_position.z - entity.position.z) > 25000.f) continue;

        auto& base = use_instance(meshes.small_stone_bridge_base);
        auto& columns = use_instance(meshes.small_stone_bridge_columns);
        const float age = state.astro_time - entity.astro_start_time;

        base.position = entity.position;
        base.scale = entity.scale;
        base.rotation = entity.orientation;
        base.color = tVec3f(0.6f, 0.5f, 0.4f);
        base.material = tVec4f(1.f, 0, 0, 0);

        columns.position = entity.position;
        columns.scale = entity.scale;
        columns.rotation = entity.orientation;
        columns.color = tVec3f(0.6f, 0.5f, 0.4f);
        columns.material = tVec4f(1.f, 0, 0, 0);

        if (age < 8.f) base.scale = tVec3f(0.f);
        if (age < 4.f) columns.scale.y *= 0.5f;
        if (age < 2.f) columns.scale.y *= 0.5f;
        if (age <= 0.f) columns.scale = tVec3f(0.f);

        // Collision
        entity.visible_scale = base.scale;

        commit(base);
        commit(columns);
      }

      Tachyon_UseLodByDistance(tachyon, meshes.small_stone_bridge_columns, 30000.f);
    }
  };
}