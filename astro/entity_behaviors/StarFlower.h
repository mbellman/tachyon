#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior StarFlower {
    addMeshes() {
      meshes.starflower_placeholder = MODEL_MESH("./astro/3d_models/starflower/placeholder.obj", 500);
      meshes.starflower_leaves = MODEL_MESH("./astro/3d_models/starflower/leaves.obj", 500);
      meshes.starflower_petals = MODEL_MESH("./astro/3d_models/starflower/petals.obj", 500);

      mesh(meshes.starflower_placeholder).shadow_cascade_ceiling = 2;

      mesh(meshes.starflower_leaves).type = FOLIAGE_MESH;
      mesh(meshes.starflower_leaves).shadow_cascade_ceiling = 2;

      mesh(meshes.starflower_petals).type = FOLIAGE_MESH;
      mesh(meshes.starflower_petals).shadow_cascade_ceiling = 2;
    }

    getMeshes() {
      return_meshes({
        meshes.starflower_leaves,
        meshes.starflower_petals
      });
    }

    getPlaceholderMesh() {
      return meshes.starflower_placeholder;
    }

    timeEvolve() {
      // profile("  StarFlower::timeEvolve()");

      auto& meshes = state.meshes;

      float petals_emissivity = state.is_nighttime ? 0.5f : 0.2f;

      reset_instances(meshes.starflower_leaves);
      reset_instances(meshes.starflower_petals);

      // @todo culling
      // @todo growth
      for_entities(state.starflowers) {
        auto& entity = state.starflowers[i];

        if (abs(entity.position.x - state.player_position.x) > 20000.f) continue;
        if (abs(entity.position.z - state.player_position.z) > 20000.f) continue;

        // float life_progress = GetLivingEntityProgress(state, entity, lifetime);
        // float growth = sqrtf(sinf(life_progress * t_PI));

        // Leaves
        {
          auto& leaves = use_instance(meshes.starflower_leaves);

          Sync(leaves, entity);

          leaves.color = tVec3f(0.2f, 0.6f, 0.3f);
          leaves.material = tVec4f(1.f, 0, 0, 0.8f);

          commit(leaves);
        }

        // Petals
        {
          auto& petals = use_instance(meshes.starflower_petals);

          Sync(petals, entity);

          petals.color = tVec4f(0.8f, 1.f, 1.f, petals_emissivity);
          petals.material = tVec4f(0.2f, 1.f, 0.5f, 1.f);

          commit(petals);
        }

        // Light
        {
          // if (entity.light_id == -1) {
          //   entity.light_id = create_point_light();
          // }

          // auto& light = *get_point_light(entity.light_id);

          // light.position = entity.position + tVec3f(0, 2.f * entity.scale.y, 0);
          // light.radius = 1500.f;
          // light.color = tVec3f(1.f, 0.6f, 0.6f);
          // light.glow_power = 0.f;

          // if (
          //   abs(entity.position.x - state.player_position.x) > 20000.f ||
          //   abs(entity.position.z - state.player_position.z) > 20000.f
          // ) {
          //   light.power = 0.f;
          // } else {
          //   light.power = 1.f;
          // }
        }
      }
    }
  };
}