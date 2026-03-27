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

      tVec4f colors[] = {
        tVec4f(1.f, 0.4f, 0.7f, 0.3f),
        tVec4f(1.f, 0.6f, 0.9f, 0.3f),
        tVec4f(1.f, 0.3f, 0.5f, 0.3f)
      };

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

          leaves.color = tVec3f(0.4f, 0.8f, 0.2f);
          leaves.material = tVec4f(0.8f, 0, 0, 0.5f);

          leaves.scale *= Grow(20.f * life_progress);

          if (life_progress > 0.8f) {
            leaves.scale.y *= 1.f - Grow(10.f * (life_progress - 0.8f));
          }

          commit(leaves);
        }

        // Stalk
        {
          auto& stalk = use_instance(meshes.tulip_plant_stalk);

          Sync(stalk, entity);

          stalk.material = tVec4f(0.8f, 0, 0, 0.5f);

          stalk.scale *= Grow(20.f * (life_progress - 0.1f));

          if (life_progress > 0.7f) {
            stalk.scale.y *= 1.f - Grow(10.f * (life_progress - 0.7f));
          }

          commit(stalk);
        }

        // Bulb
        {
          auto& bulb = use_instance(meshes.tulip_plant_bulb);
          int color_index = (int)abs(entity.position.x) % 3;

          Sync(bulb, entity);

          bulb.position = UnitEntityToWorldPosition(entity, tVec3f(0, 1.65f, -0.25f));
          bulb.rotation = entity.orientation * Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), -0.4f);
          bulb.scale *= 0.35f;
          bulb.color = colors[color_index];
          bulb.material = tVec4f(0.5f, 0, 0.1f, 1.f);

          if (entity.astro_start_time > astro_time_periods.past) {
            bulb.color = tVec4f(1.f, 1.f, 1.f, 0.3f);
          }

          bulb.scale *= Grow(20.f * (life_progress - 0.2f));

          if (life_progress > 0.6f) {
            // Wilting
            bulb.scale.y *= 1.f - Grow(10.f * (life_progress - 0.6f));

            // Dead
            if (life_progress > 0.7f) bulb.scale = tVec3f(0.f);
          }

          commit(bulb);
        }
      }
    }
  }
}