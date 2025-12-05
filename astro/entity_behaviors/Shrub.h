#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior Shrub {
    addMeshes() {
      meshes.shrub_placeholder = MODEL_MESH("./astro/3d_models/shrub/placeholder.obj", 500);
      meshes.shrub_leaves = MODEL_MESH("./astro/3d_models/shrub/leaves.obj", 500);

      mesh(meshes.shrub_placeholder).type = GRASS_MESH;
      mesh(meshes.shrub_leaves).type = GRASS_MESH;
    }

    getMeshes() {
      return_meshes({
        meshes.shrub_leaves
      });
    }

    getPlaceholderMesh() {
      return meshes.shrub_placeholder;
    }

    timeEvolve() {
      profile("  Shrub::timeEvolve()");

      auto& meshes = state.meshes;
      const float lifetime = 100.f;

      const tVec3f leaves_color = tVec3f(0.1f, 0.3f, 0.1f);

      for_entities(state.shrubs) {
        auto& entity = state.shrubs[i];
        float life_progress = GetLivingEntityProgress(state, entity, lifetime);

        // @todo factor
        auto& leaves = objects(meshes.shrub_leaves)[i];

        leaves.scale = entity.scale * sinf(life_progress * t_PI);

        if (life_progress == 0.f || life_progress == 1.f) {
          // Not yet started growing, or dead
          leaves.scale = tVec3f(0.f);
        }
        else if (life_progress > 0.5f) {
          leaves.scale.x = entity.scale.x;
          leaves.scale.z = entity.scale.z;
        }

        leaves.position = entity.position;
        leaves.position.y -= (entity.scale.y - leaves.scale.y);

        leaves.rotation = entity.orientation;
        leaves.color = 0x1210;
        leaves.material = tVec4f(0.7f, 0, 0, 0.2f);

        // Collision
        entity.visible_position = leaves.position;
        entity.visible_scale = leaves.scale;
        entity.visible_rotation = leaves.rotation;

        commit(leaves);
      }
    }
  };
}