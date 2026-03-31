#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior WillowTree {
    addMeshes() {
      meshes.willow_tree_placeholder = MODEL_MESH("./astro/3d_models/willow_tree/placeholder.obj", 500);
      meshes.willow_tree_trunk = MODEL_MESH("./astro/3d_models/willow_tree/trunk.obj", 500);
      meshes.willow_tree_branches = MODEL_MESH("./astro/3d_models/willow_tree/branches.obj", 500);
      meshes.willow_tree_leaves = MODEL_MESH("./astro/3d_models/willow_tree/leaves.obj", 500);

      mesh(meshes.willow_tree_trunk).shadow_cascade_ceiling = 2;
      mesh(meshes.willow_tree_branches).shadow_cascade_ceiling = 2;

      mesh(meshes.willow_tree_leaves).type = FOLIAGE_MESH;
      mesh(meshes.willow_tree_leaves).shadow_cascade_ceiling = 2;
    }

    getMeshes() {
      return_meshes({
        meshes.willow_tree_trunk,
        meshes.willow_tree_branches,
        meshes.willow_tree_leaves
      });
    }

    getPlaceholderMesh() {
      return meshes.willow_tree_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      const float lifetime = 200.f;

      const tVec3f wood_color = tVec3f(0.1f, 0.f, 0.f);
      const tVec3f aged_wood_color = tVec3f(0.2f, 0.2f, 0.2f);
      const tVec3f leaves_color = tVec3f(0.3f, 0.5f, 0.1f);
      const tVec3f aged_leaves_color = tVec3f(0.4f, 0.5f, 0.3f);

      float alpha = Tachyon_InverseLerp(astro_time_periods.past, astro_time_periods.present, state.astro_time);
      tVec3f current_wood_color = tVec3f::lerp(wood_color, aged_wood_color, alpha);
      tVec3f current_leaves_color = tVec3f::lerp(leaves_color, aged_leaves_color, alpha);

      reset_instances(meshes.willow_tree_trunk);
      reset_instances(meshes.willow_tree_branches);
      reset_instances(meshes.willow_tree_leaves);

      for_entities(state.willow_trees) {
        auto& entity = state.willow_trees[i];

        if (abs(state.player_position.x - entity.position.x) > 25000.f) continue;
        if (abs(state.player_position.z - entity.position.z) > 25000.f) continue;

        float life_progress = GetLivingEntityProgress(state, entity, lifetime);
        float growth_factor = 0.f;

        if (life_progress > 0.f) {
          float entity_age = state.astro_time - entity.astro_start_time;

          growth_factor = sqrtf(1.f - expf(-0.01f * entity_age));
        }

        // Trunk
        {
          auto& trunk = use_instance(meshes.willow_tree_trunk);
          float trunk_height = 1.f - powf(1.f - life_progress, 4.f);
          float trunk_thickness = 0.1f + 0.9f * -(cosf(t_PI * life_progress) - 1.f) / 2.f;

          trunk.scale = entity.scale * growth_factor;

          trunk.position = entity.position;
          trunk.position.y = entity.position.y - entity.scale.y * (1.f - trunk_height);

          trunk.rotation = entity.orientation;
          trunk.color = current_wood_color;
          trunk.material = tVec4f(1.f, 0, 0, 0.2f);

          commit(trunk);

          // Collision
          // @todo move below
          entity.visible_scale = trunk.scale;
        }

        // Branches
        {
          auto& branches = use_instance(meshes.willow_tree_branches);

          Sync(branches, entity);

          branches.scale = entity.scale * growth_factor;
          branches.color = current_wood_color;
          branches.material = tVec4f(1.f, 0, 0, 0.2f);

          commit(branches);
        }

        // Leaves
        {
          auto& leaves = use_instance(meshes.willow_tree_leaves);

          Sync(leaves, entity);

          leaves.position.y += entity.visible_scale.y * 0.15f;
          leaves.scale = entity.scale * growth_factor;
          leaves.color = tVec4f(current_leaves_color, 0.5f);
          leaves.material = tVec4f(0.8f, 0, 0, 1.f);

          commit(leaves);
        }
      }
    }
  };
}