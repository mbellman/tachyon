#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior WindChimes {
    addMeshes() {
      meshes.wind_chimes_placeholder = MODEL_MESH("./astro/3d_models/wind_chimes/placeholder.obj", 500);
      meshes.wind_chimes_stand = MODEL_MESH("./astro/3d_models/wind_chimes/stand.obj", 500);
    }

    getMeshes() {
      return_meshes({
        meshes.wind_chimes_stand
      });
    }

    getPlaceholderMesh() {
      return meshes.wind_chimes_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;
      const float lifetime = 100.f;

      for_entities(state.wind_chimes) {
        auto& entity = state.wind_chimes[i];

        // Stand
        {
          auto& stand = objects(meshes.wind_chimes_stand)[i];

          Sync(stand, entity);

          stand.color = tVec3f(1.f, 0.9f, 0.4f);
          stand.material = tVec4f(0.4f, 1.f, 0, 0);

          commit(stand);
        }
      }
    }
  };
}