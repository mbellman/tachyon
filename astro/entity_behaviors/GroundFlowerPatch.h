
#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior GroundFlowerPatch {
    addMeshes() {
      meshes.ground_flower_patch_placeholder = MODEL_MESH("./astro/3d_models/ground_flower_patch/placeholder.obj", 500);
      meshes.ground_flower_patch = MODEL_MESH("./astro/3d_models/ground_flower_patch/patch.obj", 500);

      mesh(meshes.ground_flower_patch_placeholder).shadow_cascade_ceiling = 0;

      mesh(meshes.ground_flower_patch).type = FOLIAGE_MESH;
      mesh(meshes.ground_flower_patch).shadow_cascade_ceiling = 1;
    }

    getMeshes() {
      return_meshes({
        meshes.ground_flower_patch
      });
    }

    getPlaceholderMesh() {
      return meshes.ground_flower_patch_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      reset_instances(meshes.ground_flower_patch);

      for (auto& entity : state.ground_flower_patches) {
        if (!IsInRangeX(entity, state, 20000.f)) continue;
        if (!IsInRangeZ(entity, state, 20000.f)) continue;

        // Flower patch
        {
          auto& patch = use_instance(meshes.ground_flower_patch);

          Sync(patch, entity);

          commit(patch);
        }
      }
    }
  };
}
