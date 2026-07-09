
#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior CastleRampart {
    addMeshes() {
      meshes.castle_rampart_placeholder = MODEL_MESH("./astro/3d_models/castle_rampart/placeholder.obj", 500);
      meshes.castle_rampart = MODEL_MESH("./astro/3d_models/castle_rampart/rampart.obj", 500);
      meshes.castle_rampart_frills = MODEL_MESH("./astro/3d_models/castle_rampart/frills.obj", 500);

      mesh(meshes.castle_rampart_frills).shadow_cascade_ceiling = 1;
    }

    getMeshes() {
      return_meshes({
        meshes.castle_rampart,
        meshes.castle_rampart_frills
      });
    }

    getPlaceholderMesh() {
      return meshes.castle_rampart_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      reset_instances(meshes.castle_rampart);
      reset_instances(meshes.castle_rampart_frills);

      for_entities(state.castle_ramparts) {
        auto& entity = state.castle_ramparts[i];

        // Rampart
        {
          auto& rampart = use_instance(meshes.castle_rampart);

          Sync(rampart, entity);

          rampart.material = tVec4f(0.6f, 0, 0, 1.f);

          commit(rampart);
        }

        // Skip additional details for out-of-range entities
        if (!IsInRangeX(entity, state, 25000.f)) continue;
        if (!IsInRangeZ(entity, state, 30000.f)) continue;

        // Frills
        {
          auto& frills = use_instance(meshes.castle_rampart_frills);

          Sync(frills, entity);

          frills.material = tVec4f(0.6f, 0, 0, 1.f);

          commit(frills);
        }
      }
    }
  };
}
