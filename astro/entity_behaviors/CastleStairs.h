#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior CastleStairs {
    addMeshes() {
      meshes.castle_stairs_placeholder = MODEL_MESH("./astro/3d_models/castle_stairs/placeholder.obj", 500);
      meshes.castle_stairs_platform = MODEL_MESH("./astro/3d_models/castle_stairs/platform.obj", 500);
      // @todo
      meshes.castle_stairs_step = CUBE_MESH(500);
    }

    getMeshes() {
      return_meshes({
        meshes.castle_stairs_platform
      });
    }

    getPlaceholderMesh() {
      return meshes.castle_stairs_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      reset_instances(meshes.castle_stairs_platform);

      for (auto& entity : state.castle_stairs) {
        if (!IsDuringActiveTime(entity, state)) continue;
        // @todo range culling

        // Platform
        {
          auto& platform = use_instance(meshes.castle_stairs_platform);

          Sync(platform, entity);

          commit(platform);
        }

        // Procedural stair steps
        {
          // @todo
        }
      }
    }
  };
}
