#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior TallEvergreenShrub {
    addMeshes() {
      meshes.tall_evergreen_shrub_placeholder = MODEL_MESH("./astro/3d_models/tall_evergreen_shrub/placeholder.obj", 500);
      meshes.tall_evergreen_shrub = MODEL_MESH("./astro/3d_models/tall_evergreen_shrub/shrub.obj", 500);

      mesh(meshes.tall_evergreen_shrub).type = FOLIAGE_MESH;
      mesh(meshes.tall_evergreen_shrub).shadow_cascade_ceiling = 3;
    }

    getMeshes() {
      return_meshes({
        meshes.tall_evergreen_shrub
      });
    }

    getPlaceholderMesh() {
      return meshes.tall_evergreen_shrub_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      reset_instances(meshes.tall_evergreen_shrub);

      for (auto& entity : state.tall_evergreen_shrubs) {
        if (!IsDuringActiveTime(entity, state)) continue;
        if (!IsInRangeX(entity, state, 30000.f)) continue;
        if (!IsInRangeZ(entity, state, 40000.f)) continue;

        // Shrub
        {
          auto& shrub = use_instance(meshes.tall_evergreen_shrub);

          Sync(shrub, entity);

          shrub.color = tVec3f(0.f, 0.1f, 0.f);
          shrub.material = tVec4f(0.8f, 0, 0, 0.3f);

          commit(shrub);
        }
      }
    }
  };
}
