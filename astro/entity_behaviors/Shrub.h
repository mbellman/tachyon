#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior Shrub {
    static void ApplyDefaultProperties(GameEntity& entity, tObject& leaves) {
      const tVec3f leaves_color = tVec3f(0.07f, 0.14f, 0.07f);

      leaves.position = entity.position;
      leaves.position.y -= (entity.scale.y - leaves.scale.y);

      leaves.rotation = entity.orientation;
      leaves.color = leaves_color;
      leaves.material = tVec4f(0.7f, 0, 0, 0.2f);
    }

    static float GetGrowthFactor(const float life_progress, const float offset) {
      float growth_factor = sinf((life_progress + offset) * t_PI);

      if (life_progress == 0.f || life_progress == 1.f) {
        growth_factor = 0.f;
      }

      return growth_factor;
    }

    addMeshes() {
      meshes.shrub_placeholder = MODEL_MESH("./astro/3d_models/shrub/placeholder.obj", 500);
      meshes.shrub_bottom = MODEL_MESH("./astro/3d_models/shrub/bottom.obj", 500);
      meshes.shrub_middle = MODEL_MESH("./astro/3d_models/shrub/middle.obj", 500);
      meshes.shrub_top = MODEL_MESH("./astro/3d_models/shrub/top.obj", 500);

      mesh(meshes.shrub_placeholder).type = FOLIAGE_MESH;
      mesh(meshes.shrub_placeholder).shadow_cascade_ceiling = 2;

      mesh(meshes.shrub_bottom).type = FOLIAGE_MESH;
      mesh(meshes.shrub_bottom).shadow_cascade_ceiling = 2;

      mesh(meshes.shrub_middle).type = FOLIAGE_MESH;
      mesh(meshes.shrub_middle).shadow_cascade_ceiling = 2;

      mesh(meshes.shrub_top).type = FOLIAGE_MESH;
      mesh(meshes.shrub_top).shadow_cascade_ceiling = 2;
    }

    getMeshes() {
      return_meshes({
        meshes.shrub_bottom,
        meshes.shrub_middle,
        meshes.shrub_top
      });
    }

    getPlaceholderMesh() {
      return meshes.shrub_placeholder;
    }

    timeEvolve() {
      profile("  Shrub::timeEvolve()");

      auto& meshes = state.meshes;

      const float lifetime = 100.f;
      const tVec3f leaves_color = tVec3f(0.07f, 0.14f, 0.07f);

      uint16 shrub_index = 0;

      for_entities(state.shrubs) {
        auto& entity = state.shrubs[i];

        if (abs(state.player_position.x - entity.position.x) > 25000.f) continue;
        if (abs(state.player_position.z - entity.position.z) > 25000.f) continue;

        float life_progress = GetLivingEntityProgress(state, entity, lifetime);

        // Bottom
        {
          auto& bottom = objects(meshes.shrub_bottom)[shrub_index];

          bottom.scale = entity.scale * GetGrowthFactor(life_progress, 0.f);

          // @todo factor
          if (life_progress > 0.5f && life_progress != 1.f) {
            bottom.scale.x = entity.scale.x;
            bottom.scale.z = entity.scale.z;
          }

          ApplyDefaultProperties(entity, bottom);

          commit(bottom);
        }

        // Middle
        {
          auto& middle = objects(meshes.shrub_middle)[shrub_index];

          middle.scale = entity.scale * GetGrowthFactor(life_progress, -0.15f);

          // @todo factor
          if (life_progress > 0.5f && life_progress != 1.f) {
            middle.scale.x = entity.scale.x;
            middle.scale.z = entity.scale.z;
          }

          ApplyDefaultProperties(entity, middle);

          commit(middle);
        }

        // Top
        {
          auto& top = objects(meshes.shrub_top)[shrub_index];

          top.scale = entity.scale * GetGrowthFactor(life_progress, -0.3f);

          // @todo factor
          if (life_progress > 0.5f && life_progress != 1.f) {
            top.scale.x = entity.scale.x;
            top.scale.z = entity.scale.z;
          }

          ApplyDefaultProperties(entity, top);

          commit(top);
        }

        // Collision
        entity.visible_position = entity.position;
        entity.visible_scale = entity.scale * GetGrowthFactor(life_progress, 0.f);
        entity.visible_rotation = entity.orientation;

        shrub_index++;
      }

      mesh(meshes.shrub_bottom).lod_1.instance_count = shrub_index;
      mesh(meshes.shrub_middle).lod_1.instance_count = shrub_index;
      mesh(meshes.shrub_top).lod_1.instance_count = shrub_index;
    }
  };
}