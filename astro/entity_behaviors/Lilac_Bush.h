#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior LilacBush {
    static inline float RandomWithinRange(float seed, float low, float high) {
      float r = fmod(abs(seed), 1.f);

      return low + r * (high - low);
    }

    addMeshes() {
      // @todo use own model for placeholder + leaves
      meshes.lilac_placeholder = MODEL_MESH("./astro/3d_models/shrub/placeholder.obj", 500);
      meshes.lilac_leaves = MODEL_MESH("./astro/3d_models/shrub/leaves.obj", 500);
      meshes.lilac_flower = MODEL_MESH("./astro/3d_models/lilac_bush/flower.obj", 3000);

      mesh(meshes.lilac_placeholder).type = FOLIAGE_MESH;
      mesh(meshes.lilac_leaves).type = FOLIAGE_MESH;
      mesh(meshes.lilac_flower).type = FOLIAGE_MESH;
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
      profile("  LilacBush::timeEvolve()");

      auto& meshes = state.meshes;

      const float lifetime = 100.f;
      const tVec3f leaves_color = tVec3f(0.07f, 0.14f, 0.07f);
      const tVec3f leaves_wilting_color = tVec3f(0.4f, 0.2f, 0.1f);

      uint16 leaves_index = 0;
      uint16 flower_index = 0;

      for_entities(state.lilac_bushes) {
        auto& entity = state.lilac_bushes[i];

        if (abs(state.player_position.x - entity.position.x) > 25000.f) continue;
        if (abs(state.player_position.z - entity.position.z) > 25000.f) continue;

        float life_progress = GetLivingEntityProgress(state, entity, lifetime);

        if (life_progress == 0.f || life_progress == 1.f) {
          entity.visible_scale = tVec3f(0.f);

          continue;
        }

        float plant_growth = sqrtf(sinf(life_progress * t_PI));

        // Leaves
        auto& leaves = objects(meshes.lilac_leaves)[leaves_index++];

        leaves.scale = entity.scale * plant_growth;
        leaves.scale.x = entity.scale.x * (life_progress > 0.5f ? 1.f : plant_growth);
        leaves.scale.y = entity.scale.y * plant_growth;
        leaves.scale.z = entity.scale.z * (life_progress > 0.5f ? 1.f : plant_growth);

        leaves.position = entity.position;
        leaves.rotation = entity.orientation;
        leaves.material = tVec4f(0.8f, 0, 0, 0.4f);

        if (life_progress < 0.5f) {
          // Sprouting
          leaves.scale.x = entity.scale.x * plant_growth;
          leaves.scale.z = entity.scale.z * plant_growth;
          leaves.color = leaves_color;
        }
        else if (life_progress < 1.f) {
          // Wilting
          float alpha = 2.f * (life_progress - 0.5f);
          alpha *= alpha;
          alpha *= alpha;

          leaves.scale.x = entity.scale.x * Tachyon_Lerpf(1.f, 0.7f, alpha);
          leaves.scale.z = entity.scale.x * Tachyon_Lerpf(1.f, 0.7f, alpha);
          leaves.color = tVec3f::lerp(leaves_color, leaves_wilting_color, alpha);
        }

        commit(leaves);

        // Flowers
        {
          for (int j = 0; j < 6; j++) {
            auto& flower = objects(meshes.lilac_flower)[flower_index++];
            float alpha = float(j) / 5.f;

            float flower_growth = powf(sinf(life_progress * t_PI + alpha * 0.5f), 3.f);
            tVec3f flower_scale = entity.scale * 2.f * flower_growth;
            float flower_scale_y = entity.scale.y * 2.f * sqrtf(flower_growth);

            float x_seed = entity.position.x + 3.89f * alpha;
            float z_seed = entity.position.z + 2.67f * alpha;

            float range_x = entity.scale.x * 1.2f;
            float range_z = entity.scale.z * 1.2f;

            tVec3f random_offset = tVec3f(
              RandomWithinRange(x_seed, -range_x, range_x),
              entity.visible_scale.y * (0.45f + alpha * 0.1f),
              RandomWithinRange(z_seed, -range_z, range_z)
            );

            flower.position = entity.position + random_offset;
            flower.scale = flower_scale;
            flower.scale.y = flower_scale_y;
            flower.color = tVec3f(1.f, 0.4f, 1.f);
            flower.material = tVec4f(0.8f, 0, 0, 0.6f);

            // @todo optimize
            Quaternion random_rotation =
              Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), flower.position.x + alpha) *
              Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), 0.4f);

            flower.rotation = entity.orientation * random_rotation;

            commit(flower);
          }
        }

        // Collision
        entity.visible_position = entity.position;
        entity.visible_scale = leaves.scale;
        entity.visible_rotation = entity.orientation;
      }

      mesh(meshes.lilac_leaves).lod_1.instance_count = leaves_index;
      mesh(meshes.lilac_flower).lod_1.instance_count = flower_index;
    }
  };
}