
#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior CastleTower {
    addMeshes() {
      meshes.castle_tower_placeholder = MODEL_MESH("./astro/3d_models/castle_tower/placeholder.obj", 500);
      meshes.castle_tower = MODEL_MESH("./astro/3d_models/castle_tower/tower.obj", 500);
    }

    getMeshes() {
      return_meshes({
        meshes.castle_tower
      });
    }

    getPlaceholderMesh() {
      return meshes.castle_tower_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      reset_instances(meshes.castle_tower);

      for_entities(state.castle_towers) {
        auto& entity = state.castle_towers[i];

        auto& tower = use_instance(meshes.castle_tower);

        Sync(tower, entity);

        tower.material = tVec4f(0.6f, 0, 0, 1.f);

        commit(tower);
      }
    }
  };
}
