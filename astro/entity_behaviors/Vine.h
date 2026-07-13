#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior Vine {
    addMeshes() {
      meshes.vine_placeholder = MODEL_MESH("./astro/3d_models/vine/placeholder.obj", 500);
      meshes.vine_leaves = MODEL_MESH("./astro/3d_models/vine/placeholder.obj", 500);

      mesh(meshes.vine_leaves).type = GRASS_MESH;
      mesh(meshes.vine_leaves).shadow_cascade_ceiling = 2;
    }

    getMeshes() {
      return_meshes({
        meshes.vine_leaves
      });
    }

    getPlaceholderMesh() {
      return meshes.vine_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      reset_instances(meshes.vine_leaves);

      for (auto& entity : state.vines) {
        if (!IsInRangeX(entity, state, 30000.f)) continue;
        if (!IsInRangeZ(entity, state, 40000.f)) continue;

        // Leaves
        {
          auto& leaves = use_instance(meshes.vine_leaves);

          Sync(leaves, entity);

          leaves.color = tVec3f(0.14f, 0.4f, 0.07f);
          leaves.material = tVec4f(0.6f, 0, 0, 0.5f);

          commit(leaves);
        }
      }
    }
  };
}
