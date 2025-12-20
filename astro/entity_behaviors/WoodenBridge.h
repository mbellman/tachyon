#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior WoodenBridge {
    addMeshes() {
      meshes.wooden_bridge_placeholder = MODEL_MESH("./astro/3d_models/wooden_bridge/placeholder.obj", 500);
      meshes.wooden_bridge_platform = MODEL_MESH("./astro/3d_models/wooden_bridge/platform.obj", 500);
    }

    getMeshes() {
      return_meshes({
        meshes.wooden_bridge_platform
      });
    }

    getPlaceholderMesh() {
      return meshes.wooden_bridge_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      for_entities(state.wooden_bridges) {
        auto& entity = state.wooden_bridges[i];

        // Platform
        // @temporary
        {
          auto& platform = objects(meshes.wooden_bridge_platform)[i];

          Sync(platform, entity);

          platform.color = tVec3f(1.f, 0.5f, 0.2f);
          platform.material = tVec4f(1.f, 0, 0, 0.1f);

          commit(platform);
        }

        entity.visible_position = entity.position;
        entity.visible_rotation = entity.orientation;
        // @temporary
        entity.visible_scale = entity.scale;
      }
    }
  };
}