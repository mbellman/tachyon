#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior OakTree {
    addMeshes() {
      meshes.oak_tree_placeholder = MODEL_MESH("./astro/3d_models/oak_tree/placeholder.obj", 500);
      meshes.oak_tree_roots = MODEL_MESH("./astro/3d_models/oak_tree/roots.obj", 500);
      meshes.oak_tree_trunk = MODEL_MESH("./astro/3d_models/oak_tree/trunk.obj", 500);
      meshes.oak_tree_branches = MODEL_MESH("./astro/3d_models/oak_tree/branches.obj", 500);
      meshes.oak_tree_leaves = MODEL_MESH_LOD_2("./astro/3d_models/oak_tree/leaves.obj", "./astro/3d_models/oak_tree/leaves_lod.obj", 500);

      mesh(meshes.oak_tree_roots).shadow_cascade_ceiling = 2;
      mesh(meshes.oak_tree_trunk).shadow_cascade_ceiling = 2;
      mesh(meshes.oak_tree_branches).shadow_cascade_ceiling = 2;

      mesh(meshes.oak_tree_leaves).type = FOLIAGE_MESH;
      mesh(meshes.oak_tree_leaves).shadow_cascade_ceiling = 2;
      mesh(meshes.oak_tree_leaves).use_lowest_lod_for_shadows = true;
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

      reset_instances(meshes.oak_tree_roots);
      reset_instances(meshes.oak_tree_trunk);
      reset_instances(meshes.oak_tree_branches);
      reset_instances(meshes.oak_tree_leaves);

      for_entities(state.oak_trees) {
        auto& entity = state.oak_trees[i];

        if (abs(state.player_position.x - entity.position.x) > 30000.f) continue;
        if (entity.position.z - state.player_position.z > 12000.f) continue;
        if (state.player_position.z - entity.position.z > 30000.f) continue;

        float life_progress = GetLivingEntityProgress(state, entity, lifetime);
        float growth_factor = 0.f;

        if (life_progress > 0.f) {
          float entity_age = state.astro_time - entity.astro_start_time;

          growth_factor = sqrtf(1.f - expf(-0.01f * entity_age));
        }

        float tree_height = 1.f - powf(1.f - growth_factor, 4.f);
        float tree_thickness = -(cosf(t_PI * growth_factor) - 1.f) / 2.f;

        tVec3f tree_scale = entity.scale * tVec3f(
          tree_thickness,
          tree_height,
          tree_thickness
        );

        // Roots
        {
          auto& roots = use_instance(meshes.oak_tree_roots);

          roots.scale = tree_scale;
          roots.scale.y *= 2.5f;

          roots.position = entity.position;
          roots.position.y = entity.position.y + entity.scale.y * 1.2f * tree_thickness;// - entity.scale.y * (1.f - tree_thickness);

          roots.rotation = entity.orientation;
          roots.color = wood_color;
          roots.material = wood_material;

          commit(roots);
        }

        // Trunk
        {
          auto& trunk = use_instance(meshes.oak_tree_trunk);

          trunk.scale = tree_scale;

          trunk.position = entity.position;
          trunk.position.y = entity.position.y - entity.scale.y * (1.f - tree_thickness);

          trunk.rotation = entity.orientation;
          trunk.color = wood_color;
          trunk.material = wood_material;

          commit(trunk);
        }

        // Branches
        {
          auto& branches = use_instance(meshes.oak_tree_branches);
          float branches_size = growth_factor > 0.5f ? 2.f * (growth_factor - 0.5f) : 0.f;

          branches.position = entity.position;

          branches.scale = tree_scale;

          branches.rotation = entity.orientation;
          branches.color = wood_color;
          branches.material = wood_material;

          commit(branches);
        }

        // Leaves
        {
          auto& leaves = use_instance(meshes.oak_tree_leaves);

          leaves.position = entity.position;
          leaves.position.y += entity.scale.y * 0.1f;
          leaves.scale = tree_scale;
          leaves.scale.x = tree_scale.x * 1.2f;
          leaves.scale.z = tree_scale.z * 1.2f;
          leaves.rotation = entity.orientation;
          leaves.color = leaves_color;
          leaves.material = tVec4f(0.8f, 0, 0, 1.f);

          if (state.is_nighttime) {
            // @todo use different leaf colors in Faeries' Domain
            // leaves.color = tVec4f(0.4f, 0.6f, 1.f, 0.5f);
          }

          commit(leaves);
        }

        // Collision
        entity.visible_position = entity.position;
        entity.visible_scale = tree_scale;
        entity.visible_rotation = entity.orientation;
      }
    }
  };
}