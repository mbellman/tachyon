
#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior CastleRampart {
    addMeshes() {
      meshes.castle_rampart_placeholder = MODEL_MESH("./astro/3d_models/castle_rampart/placeholder.obj", 500);
      meshes.castle_rampart = MODEL_MESH("./astro/3d_models/castle_rampart/rampart.obj", 500);
    }

    getMeshes() {
      return_meshes({
        meshes.castle_rampart
      });
    }

    getPlaceholderMesh() {
      return meshes.castle_rampart_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      reset_instances(meshes.castle_rampart);

      for_entities(state.castle_ramparts) {
        auto& entity = state.castle_ramparts[i];

        auto& rampart = use_instance(meshes.castle_rampart);

        Sync(rampart, entity);

        rampart.material = tVec4f(0.6f, 0, 0, 1.f);

        commit(rampart);
      }
    }
  };
}
