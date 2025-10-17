#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior Flowers {
    addMeshes() {
      meshes.flowers_placeholder = MODEL_MESH("./astro/3d_models/flowers/placeholder.obj", 500);
      meshes.flowers_stalks = MODEL_MESH("./astro/3d_models/flowers/stalks.obj", 500);
      meshes.flowers_petals = MODEL_MESH("./astro/3d_models/flowers/petals.obj", 500);
    }

    getMeshes() {
      return_meshes({
        meshes.flowers_stalks,
        meshes.flowers_petals
      });
    }

    getPlaceholderMesh() {
      return meshes.flowers_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;
      const float lifetime = 100.f;

      for_entities(state.flowers) {
        auto& entity = state.flowers[i];
        float life_progress = GetLivingEntityProgress(state, entity, lifetime);
        float growth = sqrtf(sinf(life_progress * t_PI));

        // @todo factor
        auto& stalks = objects(meshes.flowers_stalks)[i];

        stalks.scale = entity.scale * growth;
        stalks.position = entity.position;
        stalks.rotation = entity.orientation;
        stalks.color = entity.tint;
        stalks.material = tVec4f(0.8f, 0, 0, 0.6f);

        // @todo factor
        auto& petals = objects(meshes.flowers_petals)[i];

        petals.scale = entity.scale * growth;
        petals.position = entity.position;
        petals.rotation = entity.orientation;

        {
          // @todo make dynamic
          tVec3f blossom_color = tVec3f(1.f, 0.3f, 0.5f);
          tVec3f wilting_color = tVec3f(0.4f, 0.3f, 0.1f);
          tVec3f final_color;

          if (life_progress < 0.5f) {
            final_color = tVec3f::lerp(entity.tint, blossom_color, 2.f * life_progress);
          } else {
            float alpha = 4.f * (life_progress - 0.5f);
            if (alpha > 1.f) alpha = 1.f;

            final_color = tVec3f::lerp(blossom_color, wilting_color, alpha);
          }

          petals.color = final_color;
        }

        petals.material = tVec4f(0.9f, 0, 0, 1.f);

        // Collision
        entity.visible_scale = stalks.scale;

        commit(stalks);
        commit(petals);
      }
    }
  };
}