#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior FlowerBush {
    addMeshes() {
      meshes.flower_bush_placeholder = MODEL_MESH("./astro/3d_models/flower_bush/placeholder.obj", 2000);
      meshes.flower_bush_leaves = MODEL_MESH("./astro/3d_models/flower_bush/leaves.obj", 2000);
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
      profile("  FlowerBush::timeEvolve()");

      auto& meshes = state.meshes;
      const float lifetime = 100.f;

      const tVec3f sprout_color = tVec3f(0.2f, 0.7f, 0.3f);
      const tVec3f sprouted_color = tVec3f(0.1f, 0.4f, 0.2f);
      const tVec3f wilting_color = tVec3f(0.4f, 0.2f, 0.1f);

      for_entities(state.flower_bushes) {
        auto& entity = state.flower_bushes[i];
        float life_progress = GetLivingEntityProgress(state, entity, lifetime);
        float growth = sqrtf(sinf(life_progress * t_PI));

        // @todo factor
        auto& leaves = objects(meshes.flower_bush_leaves)[i];

        leaves.scale = entity.scale * growth;
        leaves.position = entity.position;
        leaves.rotation = entity.orientation;
        leaves.color = sprouted_color;
        leaves.material = tVec4f(0.8f, 0, 0, 0.4f);

        if (life_progress < 0.5f) {
          // Sprouting
          leaves.scale = entity.scale * growth;
          leaves.color = tVec3f::lerp(sprout_color, sprouted_color, 2.f * life_progress);
        }
        else if (life_progress < 1.f) {
          // Wilting
          float alpha = 2.f * (life_progress - 0.5f);
          alpha *= alpha;
          alpha *= alpha;

          leaves.scale.x = entity.scale.x * Tachyon_Lerpf(1.f, 0.7f, alpha);
          leaves.scale.z = entity.scale.x * Tachyon_Lerpf(1.f, 0.7f, alpha);
          leaves.color = tVec3f::lerp(sprouted_color, wilting_color, alpha);
        }
        else {
          // Dead
          leaves.scale = tVec3f(0.f);
        }

        // @todo handle flowers here, rather than in procedural_generation.cpp

        entity.visible_scale = leaves.scale;
        entity.visible_position = entity.position;
        entity.visible_rotation = entity.orientation;

        commit(leaves);
        // commit(petals);
      }
    }
  };
}