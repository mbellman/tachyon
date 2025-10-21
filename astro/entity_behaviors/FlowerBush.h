#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior FlowerBush {
    addMeshes() {
      meshes.flower_bush_placeholder = MODEL_MESH("./astro/3d_models/flower_bush/placeholder.obj", 500);
      meshes.flower_bush_leaves = MODEL_MESH("./astro/3d_models/flower_bush/leaves.obj", 500);
    }

    getMeshes() {
      return_meshes({
        meshes.flower_bush_leaves
      });
    }

    getPlaceholderMesh() {
      return meshes.flower_bush_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;
      const float lifetime = 100.f;

      for_entities(state.flower_bushes) {
        auto& entity = state.flower_bushes[i];
        float life_progress = GetLivingEntityProgress(state, entity, lifetime);
        float growth = sqrtf(sinf(life_progress * t_PI));

        // @todo factor
        auto& leaves = objects(meshes.flower_bush_leaves)[i];

        leaves.scale = entity.scale * growth;
        leaves.position = entity.position;
        leaves.rotation = entity.orientation;
        leaves.color = entity.tint;
        leaves.material = tVec4f(0.8f, 0, 0, 0.6f);

        // @todo factor
        // auto& petals = objects(meshes.flowers_petals)[i];

        // petals.scale = entity.scale * growth;
        // petals.position = entity.position;
        // petals.rotation = entity.orientation;

        // {
        //   // @todo make dynamic
        //   tVec3f blossom_color = tVec3f(1.f, 0.3f, 0.5f);
        //   tVec3f wilting_color = tVec3f(0.4f, 0.3f, 0.1f);
        //   tVec3f final_color;

        //   if (life_progress < 0.5f) {
        //     final_color = tVec3f::lerp(entity.tint, blossom_color, 2.f * life_progress);
        //   } else {
        //     float alpha = 4.f * (life_progress - 0.5f);
        //     if (alpha > 1.f) alpha = 1.f;

        //     final_color = tVec3f::lerp(blossom_color, wilting_color, alpha);
        //   }

        //   petals.color = final_color;
        // }

        // petals.material = tVec4f(0.9f, 0, 0, 1.f);

        // Collision
        entity.visible_scale = leaves.scale;

        commit(leaves);
        // commit(petals);
      }
    }
  };
}