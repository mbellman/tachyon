
#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior CastleSteeple {
    addMeshes() {
      meshes.castle_steeple_placeholder = MODEL_MESH("./astro/3d_models/castle_steeple/placeholder.obj", 500);
      meshes.castle_steeple = MODEL_MESH("./astro/3d_models/castle_steeple/steeple.obj", 500);
      meshes.castle_steeple_frills = MODEL_MESH("./astro/3d_models/castle_steeple/frills.obj", 500);

      mesh(meshes.castle_steeple_frills).shadow_cascade_ceiling = 2;
    }

    getMeshes() {
      return_meshes({
        meshes.castle_steeple,
        meshes.castle_steeple_frills
      });
    }

    getPlaceholderMesh() {
      return meshes.castle_steeple_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      reset_instances(meshes.castle_steeple);
      reset_instances(meshes.castle_steeple_frills);

      for_entities(state.castle_steeples) {
        auto& entity = state.castle_steeples[i];

        // Steeple
        {
          auto& steeple = use_instance(meshes.castle_steeple);

          Sync(steeple, entity);

          steeple.material = tVec4f(0.6f, 0, 0, 1.f);

          commit(steeple);
        }

        // Skip additional details for out-of-range entities
        if (!IsInRangeX(entity, state, 25000.f)) continue;
        if (!IsInRangeZ(entity, state, 30000.f)) continue;

        // Frills
        {
          auto& frills = use_instance(meshes.castle_steeple_frills);

          Sync(frills, entity);

          frills.material = tVec4f(0.6f, 0, 0, 1.f);

          commit(frills);
        }
      }
    }
  };
}
