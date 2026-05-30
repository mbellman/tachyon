
#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior CastleSteeple {
    addMeshes() {
      meshes.castle_steeple_placeholder = MODEL_MESH("./astro/3d_models/castle_steeple/placeholder.obj", 500);
      meshes.castle_steeple = MODEL_MESH("./astro/3d_models/castle_steeple/steeple.obj", 500);
    }

    getMeshes() {
      return_meshes({
        meshes.castle_steeple
      });
    }

    getPlaceholderMesh() {
      return meshes.castle_steeple_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      reset_instances(meshes.castle_steeple);

      for_entities(state.castle_steeples) {
        auto& entity = state.castle_steeples[i];

        auto& steeple = use_instance(meshes.castle_steeple);

        Sync(steeple, entity);

        steeple.material = tVec4f(0.6f, 0, 0, 1.f);

        commit(steeple);
      }
    }
  };
}
