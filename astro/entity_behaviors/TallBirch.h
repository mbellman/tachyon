#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior TallBirch {
    addMeshes() {
      meshes.tall_birch_placeholder = MODEL_MESH("./astro/3d_models/tall_birch/placeholder.obj", 500);
      meshes.tall_birch_trunk = MODEL_MESH("./astro/3d_models/tall_birch/trunk.obj", 500);
      meshes.tall_birch_leaves = MODEL_MESH("./astro/3d_models/tall_birch/leaves.obj", 500);

      mesh(meshes.tall_birch_placeholder).shadow_cascade_ceiling = 2;

      mesh(meshes.tall_birch_trunk).shadow_cascade_ceiling = 2;
      mesh(meshes.tall_birch_trunk).use_disocclusion = true;

      mesh(meshes.tall_birch_leaves).type = FOLIAGE_MESH;
      mesh(meshes.tall_birch_leaves).shadow_cascade_ceiling = 2;
      mesh(meshes.tall_birch_leaves).use_disocclusion = true;
    }

    getMeshes() {
      return_meshes({
        meshes.tall_birch_trunk,
        meshes.tall_birch_leaves
      });
    }

    getPlaceholderMesh() {
      return meshes.tall_birch_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      reset_instances(meshes.tall_birch_trunk);
      reset_instances(meshes.tall_birch_leaves);

      for (auto& entity : state.tall_birches) {
        if (!IsInRangeX(entity, state, 20000.f)) continue;
        if (!IsInRangeZ(entity, state, 20000.f)) continue;

        // Trunk + branches
        {
          auto& trunk = use_instance(meshes.tall_birch_trunk);

          Sync(trunk, entity);

          trunk.color = tVec4f(1.f, 1.f, 1.f, 0.4f);
          trunk.material = tVec4f(0.8f, 0, 0, 1.f);

          commit(trunk);
        }

        // Leaves
        {
          auto& leaves = use_instance(meshes.tall_birch_leaves);

          Sync(leaves, entity);

          leaves.color = tVec4f(0.5f, 0.6f, 0.1f, 0.2f);
          leaves.material = tVec4f(0.8f, 0, 0, 1.f);

          commit(leaves);
        }
      }
    }
  };
}
