#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior Ladder {
    addMeshes() {
      meshes.ladder_placeholder = MODEL_MESH("./astro/3d_models/ladder/placeholder.obj", 500);
      meshes.ladder = MODEL_MESH("./astro/3d_models/ladder/ladder.obj", 500);
    }

    getMeshes() {
      return_meshes({
        meshes.ladder
      });
    }

    getPlaceholderMesh() {
      return meshes.ladder_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      reset_instances(meshes.ladder);

      // @todo culling
      for (auto& entity : state.ladders) {
        // Ladder
        {
          auto& ladder = use_instance(meshes.ladder);

          Sync(ladder, entity);

          commit(ladder);
        }
      }
    }
  };
}
