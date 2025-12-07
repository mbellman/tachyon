#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior ChestnutTree {
    addMeshes() {
      meshes.chestnut_tree_placeholder = MODEL_MESH("./astro/3d_models/chestnut_tree/placeholder.obj", 500);
      meshes.chestnut_tree_trunk = MODEL_MESH("./astro/3d_models/chestnut_tree/trunk.obj", 500);
      meshes.chestnut_tree_leaves = MODEL_MESH("./astro/3d_models/chestnut_tree/leaves.obj", 500);

      mesh(meshes.chestnut_tree_trunk).shadow_cascade_ceiling = 3;
      mesh(meshes.chestnut_tree_leaves).shadow_cascade_ceiling = 3;

      // mesh(meshes.chestnut_tree_leaves).type = GRASS_MESH;
    }

    getMeshes() {
      return_meshes({
        meshes.chestnut_tree_trunk,
        meshes.chestnut_tree_leaves
      });
    }

    getPlaceholderMesh() {
      return meshes.chestnut_tree_placeholder;
    }

    timeEvolve() {
      profile("  ChestnutTree::timeEvolve()");

      auto& meshes = state.meshes;

      const tVec3f wood_color = tVec3f(1.f, 0.4f, 0.2f);
      const tVec4f wood_material = tVec4f(1.f, 0, 0, 0.1f);
      const tVec3f leaves_color = tVec3f(0.3f, 0.15f, 0.1f);
      const float lifetime = 200.f;

      uint16 index = 0;

      for_entities(state.chestnut_trees) {
        auto& entity = state.chestnut_trees[i];

        if (abs(state.player_position.x - entity.position.x) > 40000.f) continue;
        if (abs(state.player_position.z - entity.position.z) > 40000.f) continue;

        float life_progress = GetLivingEntityProgress(state, entity, lifetime);
        float growth_factor = 0.f;

        if (life_progress > 0.f) {
          float entity_age = state.astro_time - entity.astro_start_time;

          growth_factor = sqrtf(1.f - expf(-0.005f * entity_age));
        }

        float tree_height = 1.f - powf(1.f - growth_factor, 4.f);
        float tree_thickness = -(cosf(t_PI * growth_factor) - 1.f) / 2.f;

        // Trunk
        auto& trunk = objects(meshes.chestnut_tree_trunk)[index];

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

        // Leaves
        auto& leaves = objects(meshes.chestnut_tree_leaves)[index];

        leaves.position = entity.position;
        leaves.position.y += entity.scale.y * 0.8f;
        leaves.scale = trunk.scale;
        leaves.rotation = entity.orientation;
        leaves.color = leaves_color;
        leaves.material = tVec4f(0.8f, 0, 0, 1.f);

        // Collision
        entity.visible_position = entity.position;
        entity.visible_scale = trunk.scale;
        entity.visible_rotation = entity.orientation;

        commit(trunk);
        commit(leaves);

        index++;
      }

      mesh(meshes.oak_tree_roots).lod_1.instance_count = index;
      mesh(meshes.chestnut_tree_trunk).lod_1.instance_count = index;
      mesh(meshes.oak_tree_branches).lod_1.instance_count = index;
      mesh(meshes.chestnut_tree_leaves).lod_1.instance_count = index;
    }
  };
}