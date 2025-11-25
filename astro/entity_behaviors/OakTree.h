#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior OakTree {
    addMeshes() {
      meshes.oak_tree_placeholder = MODEL_MESH("./astro/3d_models/oak_tree/placeholder.obj", 500);
      meshes.oak_tree_roots = MODEL_MESH("./astro/3d_models/oak_tree/roots.obj", 500);
      meshes.oak_tree_trunk = MODEL_MESH("./astro/3d_models/oak_tree/trunk.obj", 500);
      meshes.oak_tree_branches = MODEL_MESH("./astro/3d_models/oak_tree/branches.obj", 500);
      meshes.oak_tree_leaves = MODEL_MESH("./astro/3d_models/oak_tree/leaves.obj", 500);

      // mesh(meshes.oak_tree_leaves).type = GRASS_MESH;
    }

    getMeshes() {
      return_meshes({
        meshes.oak_tree_roots,
        meshes.oak_tree_trunk,
        meshes.oak_tree_branches,
        meshes.oak_tree_leaves
      });
    }

    getPlaceholderMesh() {
      return meshes.oak_tree_placeholder;
    }

    timeEvolve() {
      profile("  OakTree::timeEvolve()");

      auto& meshes = state.meshes;

      const tVec3f wood_color = tVec3f(1.f, 0.4f, 0.2f);
      const tVec4f wood_material = tVec4f(1.f, 0, 0, 0.1f);
      const tVec3f leaves_color = tVec3f(0.15f, 0.3f, 0.1f);
      const float lifetime = 200.f;

      // @todo @optimize only iterate over on-screen/in-range entities
      // once that list is built
      for_entities(state.oak_trees) {
        auto& entity = state.oak_trees[i];
        float life_progress = GetLivingEntityProgress(state, entity, lifetime);
        float growth_factor = 0.f;

        if (life_progress > 0.f) {
          float entity_age = state.astro_time - entity.astro_start_time;

          growth_factor = sqrtf(1.f - expf(-0.01f * entity_age));
        }

        float tree_height = 1.f - powf(1.f - growth_factor, 4.f);
        float tree_thickness = -(cosf(t_PI * growth_factor) - 1.f) / 2.f;

        // Roots
        auto& roots = objects(meshes.oak_tree_roots)[i];

        roots.scale = entity.scale * tVec3f(
          tree_thickness,
          tree_height * 2.5f,
          tree_thickness
        );

        roots.position = entity.position;
        roots.position.y = entity.position.y + entity.scale.y * 1.2f * tree_thickness;// - entity.scale.y * (1.f - tree_thickness);

        roots.rotation = entity.orientation;
        roots.color = wood_color;
        roots.material = wood_material;

        // Trunk
        auto& trunk = objects(meshes.oak_tree_trunk)[i];

        trunk.scale = entity.scale * tVec3f(
          tree_thickness,
          tree_height,
          tree_thickness
        );

        trunk.position = entity.position;
        trunk.position.y = entity.position.y - entity.scale.y * (1.f - tree_thickness);

        trunk.rotation = entity.orientation;
        trunk.color = wood_color;
        trunk.material = wood_material;

        // Branches
        auto& branches = objects(meshes.oak_tree_branches)[i];
        float branches_size = growth_factor > 0.5f ? 2.f * (growth_factor - 0.5f) : 0.f;

        branches.position = entity.position;

        branches.scale = entity.scale * tVec3f(
          branches_size,
          tree_height,
          branches_size
        );

        branches.rotation = entity.orientation;
        branches.color = wood_color;
        branches.material = wood_material;

        // Leaves
        auto& leaves = objects(meshes.oak_tree_leaves)[i];

        leaves.position = entity.position;
        leaves.position.y += entity.scale.y * 0.8f;
        leaves.scale = entity.visible_scale * tVec3f(branches_size);
        leaves.rotation = entity.orientation;
        leaves.color = leaves_color;
        leaves.material = tVec4f(0.8f, 0, 0, 1.f);

        // Collision
        entity.visible_position = entity.position;
        entity.visible_scale = trunk.scale;
        entity.visible_rotation = entity.orientation;

        commit(roots);
        commit(trunk);
        commit(branches);
        commit(leaves);
      }
    }
  };
}