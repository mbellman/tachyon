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

      for_entities(state.small_stone_bridges) {
        auto& bridge = state.small_stone_bridges[i];
        auto& base = objects(meshes.small_stone_bridge_base)[i];
        auto& columns = objects(meshes.small_stone_bridge_columns)[i];
        const float age = state.astro_time - bridge.astro_start_time;

        base.position = bridge.position;
        base.scale = bridge.scale;
        base.rotation = bridge.orientation;
        base.color = tVec3f(0.6f, 0.5f, 0.4f);
        base.material = tVec4f(1.f, 0, 0, 0);

        columns.position = bridge.position;
        columns.scale = bridge.scale;
        columns.rotation = bridge.orientation;
        columns.color = tVec3f(0.6f, 0.5f, 0.4f);
        columns.material = tVec4f(1.f, 0, 0, 0);

        if (age < 8.f) base.scale = tVec3f(0.f);
        if (age < 4.f) columns.scale.y *= 0.5f;
        if (age < 2.f) columns.scale.y *= 0.5f;
        if (age <= 0.f) columns.scale = tVec3f(0.f);

        // Collision
        bridge.visible_scale = base.scale;

        commit(base);
        commit(columns);
      }

      Tachyon_UseLodByDistance(tachyon, meshes.small_stone_bridge_columns, 30000.f);
    }
  };
}