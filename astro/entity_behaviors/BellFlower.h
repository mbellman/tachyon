#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior BellFlower {
    addMeshes() {
      meshes.bellflower_placeholder = MODEL_MESH("./astro/3d_models/bellflower/placeholder.obj", 500);
      meshes.bellflower_stems = MODEL_MESH("./astro/3d_models/bellflower/stems.obj", 500);
      meshes.bellflower_petals = MODEL_MESH("./astro/3d_models/bellflower/petals.obj", 500);

      mesh(meshes.bellflower_placeholder).shadow_cascade_ceiling = 2;

      mesh(meshes.bellflower_stems).type = FOLIAGE_MESH;
      mesh(meshes.bellflower_stems).shadow_cascade_ceiling = 2;

      mesh(meshes.bellflower_petals).type = FOLIAGE_MESH;
      mesh(meshes.bellflower_petals).shadow_cascade_ceiling = 2;
    }

    getMeshes() {
      return_meshes({
        meshes.bellflower_stems,
        meshes.bellflower_petals
      });
    }

    getPlaceholderMesh() {
      return meshes.bellflower_placeholder;
    }

    timeEvolve() {
      // profile("  BellFlower::timeEvolve()");

      auto& meshes = state.meshes;

      float petals_emissivity = state.is_nighttime ? 0.6f : 0.4f;
      float light_power = state.is_nighttime ? 1.f : 0.1f;

      // @todo culling
      // @todo growth
      for_entities(state.bellflowers) {
        auto& entity = state.bellflowers[i];
        // float life_progress = GetLivingEntityProgress(state, entity, lifetime);
        // float growth = sqrtf(sinf(life_progress * t_PI));

        // Stems
        {
          auto& stems = objects(meshes.bellflower_stems)[i];

          Sync(stems, entity);

          stems.color = tVec3f(0.2f, 0.6f, 0.3f);
          stems.material = tVec4f(1.f, 0, 0, 0.4f);

          commit(stems);
        }

        // Petals
        {
          auto& petals = objects(meshes.bellflower_petals)[i];

          Sync(petals, entity);

          petals.color = tVec4f(1.f, 0.6f, 0.5f, petals_emissivity);
          petals.material = tVec4f(1.f, 0, 0.2f, 1.f);

          commit(petals);
        }

        // Light
        {
          if (entity.light_id == -1) {
            entity.light_id = create_point_light();
          }

          auto& light = *get_point_light(entity.light_id);

          light.position = entity.position + tVec3f(0, 2.f * entity.scale.y, 0);
          light.radius = 1500.f;
          light.color = tVec3f(1.f, 0.6f, 0.6f);
          light.glow_power = 0.f;

          if (
            abs(entity.position.x - state.player_position.x) > 20000.f ||
            abs(entity.position.z - state.player_position.z) > 20000.f
          ) {
            light.power = 0.f;
          } else {
            light.power = light_power;
          }
        }
      }
    }
  };
}