#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior BirchTree {
    addMeshes() {
      meshes.birch_tree_placeholder = MODEL_MESH("./astro/3d_models/birch_tree/placeholder.obj", 500);
      meshes.birch_tree_trunk = MODEL_MESH("./astro/3d_models/birch_tree/trunk.obj", 500);
      meshes.birch_tree_roots = MODEL_MESH("./astro/3d_models/birch_tree/roots.obj", 500);
      meshes.birch_tree_leaves = MODEL_MESH("./astro/3d_models/birch_tree/leaves.obj", 500);

      mesh(meshes.birch_tree_placeholder).shadow_cascade_ceiling = 2;
      mesh(meshes.birch_tree_trunk).shadow_cascade_ceiling = 2;
      mesh(meshes.birch_tree_roots).shadow_cascade_ceiling = 2;
      mesh(meshes.birch_tree_leaves).shadow_cascade_ceiling = 2;

      mesh(meshes.birch_tree_leaves).type = FOLIAGE_MESH;
      mesh(meshes.birch_tree_leaves).use_disocclusion = true;
    }

    getMeshes() {
      return_meshes({
        meshes.birch_tree_trunk,
        meshes.birch_tree_roots,
        meshes.birch_tree_leaves
      });
    }

    getPlaceholderMesh() {
      return meshes.birch_tree_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      const tVec3f roots_color = tVec3f(0.7f, 0.6f, 0.5f);
      const tVec3f wood_color = tVec3f(1.f);
      const tVec3f aged_wood_color = tVec3f(0.9f, 0.8f, 0.6f);

      reset_instances(meshes.birch_tree_trunk);
      reset_instances(meshes.birch_tree_roots);
      reset_instances(meshes.birch_tree_leaves);

      // @todo time evolution
      // @todo range culling
      for_entities(state.birch_trees) {
        auto& entity = state.birch_trees[i];

        // Roots
        {
          auto& roots = use_instance(meshes.birch_tree_roots);

          Sync(roots, entity);

          roots.color = roots_color;
          roots.color.rgba |= 0x0003;
          roots.material = tVec4f(1.f, 0, 0, 0.5f);

          commit(roots);
        }

        // Trunk
        {
          auto& trunk = use_instance(meshes.birch_tree_trunk);

          Sync(trunk, entity);

          trunk.color = wood_color;
          trunk.color.rgba |= 0x0003;
          trunk.material = tVec4f(1.f, 0, 0, 0.5f);

          commit(trunk);
        }

        // Leaves
        {
          auto& leaves = use_instance(meshes.birch_tree_leaves);

          Sync(leaves, entity);

          leaves.color = tVec3f(0.3f, 0.5f, 0.1f);
          leaves.material = tVec4f(0.8f, 0, 0, 1.f);

          commit(leaves);
        }
      }
    }
  };
}
