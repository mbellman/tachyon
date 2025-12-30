#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior BellFlower {
    addMeshes() {
      meshes.bellflower_placeholder = MODEL_MESH("./astro/3d_models/bellflower/placeholder.obj", 500);
      meshes.bellflower_stems = MODEL_MESH("./astro/3d_models/bellflower/stems.obj", 500);
      meshes.bellflower_petals = MODEL_MESH("./astro/3d_models/bellflower/petals.obj", 500);

      mesh(meshes.bellflower_stems).type = FOLIAGE_MESH;
      mesh(meshes.bellflower_petals).type = FOLIAGE_MESH;
    }

    getMeshes() {
      return_meshes({
        meshes.bellflower_stems,
        meshes.bellflower_petals
      });
    }

    getPlaceholderMesh() {
      return meshes.bellflower_placeholder;
    }

    timeEvolve() {
      // profile("  FlowerBush::timeEvolve()");

      auto& meshes = state.meshes;

      const tVec3f sprout_color = tVec3f(0.2f, 0.3f, 0.1f);
      const tVec3f sprouted_color = tVec3f(0.1f, 0.2f, 0.1f);
      const tVec3f wilting_color = tVec3f(0.4f, 0.2f, 0.1f);

      for_entities(state.bellflowers) {
        auto& entity = state.bellflowers[i];
        // float life_progress = GetLivingEntityProgress(state, entity, lifetime);
        // float growth = sqrtf(sinf(life_progress * t_PI));

        // Stems
        {
          auto& stems = objects(meshes.bellflower_stems)[i];

          Sync(stems, entity);

          stems.color = tVec3f(0.2f, 0.6f, 0.3f);
          stems.material = tVec4f(1.f, 0, 0, 0.4f);

          commit(stems);
        }

        // Petals
        {
          auto& petals = objects(meshes.bellflower_petals)[i];

          Sync(petals, entity);

          float emissivity = state.is_nighttime ? 0.6f : 0.4f;

          petals.color = tVec4f(1.f, 0.6f, 0.5f, emissivity);
          petals.material = tVec4f(1.f, 0, 0.2f, 1.f);

          commit(petals);
        }
      }
    }
  };
}