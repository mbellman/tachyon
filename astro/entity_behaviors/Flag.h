#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior Flag {
    addMeshes() {
      meshes.flag_placeholder = MODEL_MESH("./astro/3d_models/flag/placeholder.obj", 500);
      meshes.flag_pole = MODEL_MESH("./astro/3d_models/flag/pole.obj", 500);
      meshes.flag_banner = MODEL_MESH("./astro/3d_models/flag/banner.obj", 500);

      mesh(meshes.flag_placeholder).shadow_cascade_ceiling = 3;
      mesh(meshes.flag_pole).shadow_cascade_ceiling = 2;

      // @temporary
      // @todo CLOTH_MESH
      mesh(meshes.flag_banner).type = FOLIAGE_MESH;
      mesh(meshes.flag_banner).shadow_cascade_ceiling = 2;
    }

    getMeshes() {
      return_meshes({
        meshes.flag_pole,
        meshes.flag_banner
      });
    }

    getPlaceholderMesh() {
      return meshes.flag_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      // @todo culling
      // @todo time evolution
      for_entities(state.flags) {
        auto& entity = state.flags[i];

        // Pole
        {
          auto& pole = objects(meshes.flag_pole)[i];

          Sync(pole, entity);

          pole.color = tVec3f(0.7f, 0.5f, 0.2f);
          pole.material = tVec4f(1.f, 0, 0, 0.4f);

          commit(pole);
        }

        // Banner
        {
          auto& banner = objects(meshes.flag_banner)[i];

          Sync(banner, entity);

          banner.color = tVec4f(1.f, 0.8f, 0.4f, 0.1f);
          banner.material = tVec4f(0.7f, 0, 0, 1.f);

          commit(banner);
        }
      }
    }
  };
}