#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior WillowTree {
    addMeshes() {
      meshes.willow_tree_placeholder = MODEL_MESH("./astro/3d_models/willow_tree/placeholder.obj", 500);
      meshes.willow_tree_trunk = MODEL_MESH("./astro/3d_models/willow_tree/trunk.obj", 500);
      meshes.willow_tree_branches = MODEL_MESH("./astro/3d_models/willow_tree/branches.obj", 500);
    }

    getMeshes() {
      return_meshes({
        meshes.willow_tree_trunk,
        meshes.willow_tree_branches
      });
    }

    getPlaceholderMesh() {
      return meshes.willow_tree_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      const tVec3f trunk_color = tVec3f(0.1f, 0.f, 0.f);
      const float lifetime = 200.f;

      for_entities(state.willow_trees) {
        auto& entity = state.willow_trees[i];

        float life_progress = GetLivingEntityProgress(state, entity, lifetime);
        float growth_factor = 0.f;

        if (life_progress > 0.f) {
          float entity_age = state.astro_time - entity.astro_start_time;

          growth_factor = sqrtf(1.f - expf(-0.01f * entity_age));
        }

        // Trunk
        {
          auto& trunk = objects(meshes.willow_tree_trunk)[i];
          float trunk_height = 1.f - powf(1.f - life_progress, 4.f);
          float trunk_thickness = 0.1f + 0.9f * -(cosf(t_PI * life_progress) - 1.f) / 2.f;

          trunk.scale = entity.scale * growth_factor;

          trunk.position = entity.position;
          trunk.position.y = entity.position.y - entity.scale.y * (1.f - trunk_height);

          trunk.rotation = entity.orientation;
          trunk.color = trunk_color;
          trunk.material = tVec4f(1.f, 0, 0, 0.2f);

          commit(trunk);

          // Collision
          // @todo move below
          entity.visible_scale = trunk.scale;
        }

        // Branches
        {
          auto& branches = objects(meshes.willow_tree_branches)[i];

          Sync(branches, entity);

          branches.scale = entity.scale * growth_factor;
          branches.color = trunk_color;
          branches.material = tVec4f(1.f, 0, 0, 0.2f);

          commit(branches);
        }

      }
    }
  };
}