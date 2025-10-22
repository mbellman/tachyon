#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior GlowFlower {
    addMeshes() {
      meshes.glow_flower_placeholder = MODEL_MESH("./astro/3d_models/glow_flower/placeholder.obj", 500);
      meshes.glow_flower_petals = MODEL_MESH("./astro/3d_models/glow_flower/petals.obj", 500);
    }

    getMeshes() {
      return_meshes({
        meshes.glow_flower_petals
      });
    }

    getPlaceholderMesh() {
      return meshes.glow_flower_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;
      const float lifetime = 50.f;

      for_entities(state.glow_flowers) {
        auto& entity = state.glow_flowers[i];
        float life_progress = GetLivingEntityProgress(state, entity, lifetime);

        // @todo factor
        auto& petals = objects(meshes.glow_flower_petals)[i];

        petals.position = entity.position;
        petals.rotation = entity.orientation;
        petals.scale = entity.scale * sinf(life_progress * t_PI);
        petals.color = tVec4f(entity.tint, 0.4f);
        petals.material = tVec4f(0.4f, 1.f, 0, 0.3f);

        entity.visible_scale = petals.scale;
        entity.visible_position = entity.position;

        {
          if (entity.light_id == -1) {
            entity.light_id = create_point_light();
          }

          auto& light = *get_point_light(entity.light_id);

          light.position = entity.visible_position;
          light.position.y = entity.visible_position.y + entity.visible_scale.y * 0.25f;
          light.radius = 600.f * (entity.visible_scale.x / entity.scale.x);
          light.color = tVec3f(0.1f, 0.4f, 1.f);
          light.power = 2.f;
        }

        commit(petals);
      }
    }
  };
}