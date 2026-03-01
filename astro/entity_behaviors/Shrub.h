#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior Shrub {
    static void ApplyDefaultProperties(GameEntity& entity, tObject& leaves) {
      leaves.position = entity.position;
      leaves.position.y -= (entity.scale.y - leaves.scale.y);

      leaves.rotation = entity.orientation;
      leaves.material = tVec4f(0.7f, 0, 0, 0.2f);
    }

    static tVec3f GetPlantColor(const float life_progress) {
      const tVec3f sprouting_color = tVec3f(0.5f, 1.f, 0.5f);
      const tVec3f leaves_color = tVec3f(0.07f, 0.14f, 0.07f);
      const tVec3f wilting_color = tVec3f(0.4f, 0.2f, 0.1f);

      if (life_progress < 0.5f) {
        float alpha = life_progress / 0.5f;

        return tVec3f::lerp(sprouting_color, leaves_color, alpha);
      }
      else if (life_progress > 0.8f) {
        float alpha = (life_progress - 0.8f) / 0.2f;

        return tVec3f::lerp(leaves_color, wilting_color, alpha);
      }
      else {
        return leaves_color;
      }
    }

    static float GetGrowthFactor(const float life_progress, const float offset) {
      if (life_progress == 0.f || life_progress == 1.f) {
        return 0.f;
      }

      float growth_factor = sinf((life_progress + offset) * t_PI);

      return growth_factor < 0.f ? 0.f : growth_factor;
    }

    static float GetWidthFactor(const float life_progress, const float offset) {
      if (life_progress == 0.f || life_progress == 1.f) {
        return 0.f;
      }

      float progress = life_progress + offset;

      if (progress < 0.f) return 0.f;

      if (progress < 0.5f) {
        float p = 2.f * progress;

        return p * p;
      }

      return sqrtf(1.f - (progress - 0.5f));
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

      reset_instances(meshes.shrub_bottom);
      reset_instances(meshes.shrub_middle);
      reset_instances(meshes.shrub_top);

      for_entities(state.shrubs) {
        auto& entity = state.shrubs[i];

        if (abs(state.player_position.x - entity.position.x) > 25000.f) continue;
        if (abs(state.player_position.z - entity.position.z) > 25000.f) continue;

        float life_progress = GetLivingEntityProgress(state, entity, lifetime);
        tVec3f plant_color = GetPlantColor(life_progress);

        // Bottom
        {
          auto& bottom = use_instance(meshes.shrub_bottom);
          float growth_factor = GetGrowthFactor(life_progress, 0.f);
          float width_factor = GetWidthFactor(life_progress, 0.f);

          bottom.scale.y = entity.scale.y * growth_factor;
          bottom.scale.x = entity.scale.x * width_factor;
          bottom.scale.z = entity.scale.z * width_factor;
          bottom.color = plant_color;

          ApplyDefaultProperties(entity, bottom);

          commit(bottom);
        }

        // Middle
        {
          auto& middle = use_instance(meshes.shrub_middle);
          float growth_factor = GetGrowthFactor(life_progress, -0.15f);
          float width_factor = GetWidthFactor(life_progress, -0.15f);

          middle.scale.y = entity.scale.y * growth_factor;
          middle.scale.x = entity.scale.x * width_factor;
          middle.scale.z = entity.scale.z * width_factor;
          middle.color = plant_color;

          ApplyDefaultProperties(entity, middle);

          commit(middle);
        }

        // Top
        {
          auto& top = use_instance(meshes.shrub_top);
          float growth_factor = GetGrowthFactor(life_progress, -0.3f);
          float width_factor = GetWidthFactor(life_progress, -0.3f);

          top.scale.y = entity.scale.y * growth_factor;
          top.scale.x = entity.scale.x * width_factor;
          top.scale.z = entity.scale.z * width_factor;
          top.color = plant_color;

          ApplyDefaultProperties(entity, top);

          commit(top);
        }

        // Collision
        entity.visible_position = entity.position;
        entity.visible_scale = entity.scale * GetGrowthFactor(life_progress, 0.f);
        entity.visible_rotation = entity.orientation;
      }
    }
  };
}