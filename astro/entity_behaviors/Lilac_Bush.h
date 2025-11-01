#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  static inline float RandomWithinRange(float seed, float low, float high) {
    float r = fmod(abs(seed), 1.f);

    return low + r * (high - low);
  }

  behavior LilacBush {
    addMeshes() {
      meshes.lilac_placeholder = MODEL_MESH("./astro/3d_models/lilac_bush/placeholder.obj", 500);
      meshes.lilac_leaves = MODEL_MESH("./astro/3d_models/lilac_bush/leaves.obj", 500);
      meshes.lilac_flower = MODEL_MESH("./astro/3d_models/lilac_bush/flower.obj", 3000);

      mesh(meshes.lilac_placeholder).type = GRASS_MESH;
      mesh(meshes.lilac_leaves).type = GRASS_MESH;
      mesh(meshes.lilac_flower).type = GRASS_MESH;
    }

    getMeshes() {
      return_meshes({
        meshes.lilac_leaves,

        // Use 6 flowers per bush
        meshes.lilac_flower,
        meshes.lilac_flower,
        meshes.lilac_flower,
        meshes.lilac_flower,
        meshes.lilac_flower,
        meshes.lilac_flower
      });
    }

    getPlaceholderMesh() {
      return meshes.lilac_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;
      const float lifetime = 100.f;

      // @todo only update on-screen entities
      for_entities(state.lilac_bushes) {
        auto& entity = state.lilac_bushes[i];
        float life_progress = GetLivingEntityProgress(state, entity, lifetime);
        float plant_growth = sqrtf(sinf(life_progress * t_PI));
        float flower_growth = powf(sinf(life_progress * t_PI), 3.f);

        // @todo factor
        auto& leaves = objects(meshes.lilac_leaves)[i];

        leaves.scale = entity.scale * plant_growth;
        leaves.position = entity.position;
        leaves.rotation = entity.orientation;
        leaves.color = tVec3f(0.1f, 0.3f, 0.2f);
        leaves.material = tVec4f(0.8f, 0, 0, 0.4f);

        commit(leaves);

        // @todo factor
        tVec3f flower_scale = entity.scale * 0.5f * flower_growth;
        float flower_scale_y = entity.scale.y * 0.5f * sqrtf(flower_growth);

        for (int j = 0; j < 6; j++) {
          auto& flower = objects(meshes.lilac_flower)[i * 6 + j];
          float alpha = float(j) / 5.f;

          float range_x = entity.scale.x * 0.3f;
          float range_z = entity.scale.z * 0.3f;

          tVec3f offset = tVec3f(
            RandomWithinRange(entity.position.x + 3.89f * alpha, -range_x, range_x),
            entity.visible_scale.y * (0.45f + alpha * 0.1f),
            RandomWithinRange(entity.position.z + 2.67f * alpha, -range_z, range_z)
          );

          // @todo optimize
          Quaternion rotation =
            Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), flower.position.x + alpha) *
            Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), 0.4f);

          flower.scale = flower_scale;
          flower.scale.y = flower_scale_y;
          flower.position = entity.position + offset;
          flower.rotation = entity.orientation * rotation;
          flower.color = tVec3f(1.f, 0.4f, 1.f);
          flower.material = tVec4f(0.8f, 0, 0, 0.6f);

          commit(flower);
        }

        // Collision
        entity.visible_scale = leaves.scale;
        entity.visible_position = entity.position;
      }
    }
  };
}