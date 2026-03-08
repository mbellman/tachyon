#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior TallGrass {
    addMeshes() {
      meshes.tall_grass_placeholder = MODEL_MESH("./astro/3d_models/tall_grass/placeholder.obj", 500);
      meshes.tall_grass = MODEL_MESH("./astro/3d_models/tall_grass/grass.obj", 500);

      mesh(meshes.tall_grass_placeholder).shadow_cascade_ceiling = 2;
      mesh(meshes.tall_grass).shadow_cascade_ceiling = 2;

      mesh(meshes.tall_grass).type = GRASS_MESH;
    }

    getMeshes() {
      return_meshes({
        meshes.tall_grass
      });
    }

    getPlaceholderMesh() {
      return meshes.tall_grass_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      for_entities(state.tall_grasses) {
        auto& entity = state.tall_grasses[i];

        auto& grass = objects(meshes.tall_grass)[i];

        Sync(grass, entity);

        commit(grass);
      }
    }
  }
}