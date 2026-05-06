
#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior BirchTree {
    addMeshes() {
      meshes.birch_tree_placeholder = MODEL_MESH("./astro/3d_models/birch_tree/placeholder.obj", 500);
      meshes.birch_tree_trunk = MODEL_MESH("./astro/3d_models/birch_tree/trunk.obj", 500);

      mesh(meshes.birch_tree_placeholder).shadow_cascade_ceiling = 2;
      mesh(meshes.birch_tree_trunk).shadow_cascade_ceiling = 2;
    }

    getMeshes() {
      return_meshes({
        meshes.birch_tree_trunk
      });
    }

    getPlaceholderMesh() {
      return meshes.birch_tree_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      const tVec3f wood_color = tVec3f(1.f, 0.9f, 0.8f);
      const tVec3f aged_wood_color = tVec3f(0.9f, 0.8f, 0.6f);

      reset_instances(meshes.birch_tree_trunk);

      for_entities(state.birch_trees) {
        auto& entity = state.birch_trees[i];

        // Trunk
        {
          auto& trunk = use_instance(meshes.birch_tree_trunk);

          Sync(trunk, entity);

          trunk.color = wood_color;
          trunk.material = tVec4f(1.f, 0, 0, 0.5f);

          commit(trunk);
        }
      }
    }
  };
}
