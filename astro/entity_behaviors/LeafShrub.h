#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior LeafShrub {
    static tVec3f GetPlantColor(const tVec3f& entity_color, const float life_progress) {
      const tVec3f sprouting_color = tVec3f(0.2f, 1.f, 0.2f);
      const tVec3f leaves_color = entity_color;
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

    static float GetHeightFactor(const float life_progress, const float offset) {
      if (life_progress == 0.f || life_progress == 1.f) {
        return 0.f;
      }

      if (life_progress < 0.6f) {
        return Grow(40.f * (life_progress + offset));
      } else {
        return Die((life_progress - 0.6f) / 0.4f);
      }
    }

    static float GetWidthFactor(const float life_progress, const float offset) {
      if (life_progress == 0.f || life_progress == 1.f) {
        return 0.f;
      }

      if (life_progress < 0.8f) {
        return Grow(8.f * (life_progress + offset));
      } else {
        return 1.f - 2.f * (life_progress - 0.8f);
      }
    }

    addMeshes() {
      meshes.leaf_shrub_placeholder = MODEL_MESH("./astro/3d_models/leaf_shrub/placeholder.obj", 500);
      meshes.leaf_shrub_plant = MODEL_MESH("./astro/3d_models/leaf_shrub/plant.obj", 500);

      mesh(meshes.leaf_shrub_placeholder).type = FOLIAGE_MESH;
      mesh(meshes.leaf_shrub_placeholder).shadow_cascade_ceiling = 2;

      mesh(meshes.leaf_shrub_plant).type = FOLIAGE_MESH;
      mesh(meshes.leaf_shrub_plant).shadow_cascade_ceiling = 2;
    }

    getMeshes() {
      return_meshes({
        meshes.leaf_shrub_plant
      });
    }

    getPlaceholderMesh() {
      return meshes.leaf_shrub_placeholder;
    }

    timeEvolve() {
      // profile("  LeafShrub::timeEvolve()");

      auto& meshes = state.meshes;

      const float lifetime = 100.f;

      reset_instances(meshes.leaf_shrub_plant);

      for_entities(state.leaf_shrubs) {
        auto& entity = state.leaf_shrubs[i];

        if (abs(state.player_position.x - entity.position.x) > 20000.f) continue;
        if (abs(state.player_position.z - entity.position.z) > 20000.f) continue;

        float life_progress = GetLivingEntityProgress(state, entity, lifetime);
        tVec3f plant_color = GetPlantColor(entity.tint, life_progress);

        // Plant
        {
          auto& plant = use_instance(meshes.leaf_shrub_plant);
          float height_factor = GetHeightFactor(life_progress, 0.f);
          float width_factor = GetWidthFactor(life_progress, 0.f);

          Sync(plant, entity);

          plant.position.y -= 0.75f * entity.scale.y * (1.f - height_factor);
          plant.scale.y = entity.scale.y * height_factor;
          plant.scale.x = entity.scale.x * width_factor;
          plant.scale.z = entity.scale.z * width_factor;
          plant.color = plant_color;
          plant.material = tVec4f(0.7f, 0, 0, 0.2f);

          commit(plant);
        }

        // Collision
        entity.visible_position = entity.position;
        entity.visible_scale = entity.scale * GetHeightFactor(life_progress, 0.f);
        entity.visible_rotation = entity.orientation;
      }
    }
  };
}