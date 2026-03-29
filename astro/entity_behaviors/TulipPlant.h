#pragma once

#include "astro/entity_behaviors/behavior.h"
#include "astro/astrolabe.h"

namespace astro {
  behavior TulipPlant {
    addMeshes() {
      meshes.tulip_plant_placeholder = MODEL_MESH("./astro/3d_models/tulip_plant/placeholder.obj", 500);
      meshes.tulip_plant_leaves = MODEL_MESH("./astro/3d_models/tulip_plant/leaves.obj", 500);
      meshes.tulip_plant_stalk = MODEL_MESH("./astro/3d_models/tulip_plant/stalk.obj", 500);
      meshes.tulip_plant_bulb = MODEL_MESH("./astro/3d_models/tulip_plant/bulb.obj", 500);

      mesh(meshes.tulip_plant_placeholder).shadow_cascade_ceiling = 2;
      mesh(meshes.tulip_plant_leaves).shadow_cascade_ceiling = 2;
      mesh(meshes.tulip_plant_stalk).shadow_cascade_ceiling = 2;
      mesh(meshes.tulip_plant_bulb).shadow_cascade_ceiling = 2;

      mesh(meshes.tulip_plant_leaves).type = FOLIAGE_MESH;
      mesh(meshes.tulip_plant_stalk).type = FOLIAGE_MESH;
      mesh(meshes.tulip_plant_bulb).type = FOLIAGE_MESH;
    }

    getMeshes() {
      return_meshes({
        meshes.tulip_plant_leaves,
        meshes.tulip_plant_stalk,
        meshes.tulip_plant_bulb
      });
    }

    getPlaceholderMesh() {
      return meshes.tulip_plant_placeholder;
    }

    timeEvolve() {
      profile("  TulipPlant::timeEvolve()");

      auto& meshes = state.meshes;

      float lifetime = 100.f;

      tVec3f unique_colors[] = {
        tVec3f(1.f, 0.4f, 0.7f),
        tVec3f(1.f, 0.6f, 0.9f),
        tVec3f(1.f, 0.3f, 0.5f)
      };

      const tVec3f leaves_color = tVec3f(0.4f, 0.8f, 0.2f);
      const tVec3f wilted_color = tVec3f(0.1f, 0, 0);

      reset_instances(meshes.tulip_plant_leaves);
      reset_instances(meshes.tulip_plant_stalk);
      reset_instances(meshes.tulip_plant_bulb);

      for_entities(state.tulip_plants) {
        auto& entity = state.tulip_plants[i];

        if (abs(state.player_position.x - entity.position.x) > 20000.f) continue;
        if (abs(state.player_position.z - entity.position.z) > 20000.f) continue;

        float life_progress = GetLivingEntityProgress(state, entity, lifetime);

        if (life_progress == 0.f || life_progress == 1.f) continue;

        entity.visible_position = entity.position;
        entity.visible_rotation = entity.orientation;
        entity.visible_scale = entity.scale;

        // Leaves
        {
          auto& leaves = use_instance(meshes.tulip_plant_leaves);

          Sync(leaves, entity);

          leaves.material = tVec4f(0.8f, 0, 0, 0.5f);

          if (life_progress < 0.7f) {
            leaves.scale *= Grow(20.f * life_progress);
            leaves.color = leaves_color;
          } else {
            float death = Die((life_progress - 0.7f) / 0.3f);

            // leaves.scale.y *= Die((life_progress - 0.8f) / 0.2f);
            leaves.scale.x *= 0.5f + 0.5f * death;
            leaves.scale.y *= 0.2f + 0.8f * death;
            leaves.scale.z *= 0.5f + 0.5f * death;
            leaves.color = tVec3f::lerp(wilted_color, leaves_color, death * death);
          }

          commit(leaves);
        }

        // Stalk
        {
          auto& stalk = use_instance(meshes.tulip_plant_stalk);

          Sync(stalk, entity);

          stalk.color = leaves_color;
          stalk.material = tVec4f(0.8f, 0, 0, 0.5f);

          if (life_progress < 0.6f) {
            stalk.scale *= Grow(40.f * (life_progress - 0.1f));
          } else {
            stalk.scale.y *= Die((life_progress - 0.6f) / 0.4f);
          }

          commit(stalk);
        }

        // Bulb
        {
          auto& bulb = use_instance(meshes.tulip_plant_bulb);
          int color_index = (int)abs(entity.position.x) % 3;
          tVec3f unique_color = unique_colors[color_index];

          if (entity.astro_start_time > astro_time_periods.past) {
            // Use an off white color for tulips in the present age
            unique_color = tVec3f(1.f);
          }

          Sync(bulb, entity);

          bulb.position = UnitEntityToWorldPosition(entity, tVec3f(0, 1.65f, -0.25f));
          bulb.rotation = entity.orientation * Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), -0.4f);
          bulb.scale *= 0.35f;
          bulb.material = tVec4f(0.5f, 0, 0.1f, 1.f);

          if (life_progress < 0.55f) {
            bulb.scale *= Grow(20.f * (life_progress - 0.2f));
            bulb.color = tVec4f(unique_color, 0.3f);
          } else {
            // Wilting
            float death = Die((life_progress - 0.55f) / 0.25f);

            bulb.scale.x *= death;
            bulb.scale.y *= 0.6f + 0.4f * death;
            bulb.scale.z *= death;
            bulb.color = tVec3f::lerp(wilted_color, unique_color, death * death);

            if (life_progress > 0.6f) {
              float alpha = Die((life_progress - 0.6f) / 0.2f);

              bulb.position.y -= (1.f - alpha) * entity.scale.y;
            }

            // Dead
            if (life_progress > 0.8f) bulb.scale = tVec3f(0.f);
          }

          commit(bulb);
        }
      }
    }
  }
}