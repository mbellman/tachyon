#pragma once

#include "astro/entity_behaviors/behavior.h"
#include "astro/ui_system.h"

namespace astro {
  behavior Altar {
    addMeshes() {
      meshes.altar_placeholder = MODEL_MESH("./astro/3d_models/altar/placeholder.obj", 500);
      meshes.altar_base = MODEL_MESH("./astro/3d_models/altar/base.obj", 500);
      meshes.altar_statue = MODEL_MESH("./astro/3d_models/altar/statue.obj", 500);
    }

    getMeshes() {
      return_meshes({
        meshes.altar_base,
        meshes.altar_statue
      });
    }

    getPlaceholderMesh() {
      return meshes.altar_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      for_entities(state.altars) {
        auto& entity = state.altars[i];

        // Base
        {
          auto& base = objects(meshes.altar_base)[i];

          Sync(base, entity);

          commit(base);
        }

        // Statue
        {
          auto& statue = objects(meshes.altar_statue)[i];

          Sync(statue, entity);

          commit(statue);
        }

        // Offerings
        {
          tVec3f offering_position = UnitEntityToWorldPosition(entity, tVec3f(-0.3f, 0.4f, 0));
          float offering_distance = tVec3f::distance(state.player_position, offering_position);

          if (offering_distance < 3000.f) {
            UISystem::ShowTransientDialogue(tachyon, state, "[X] Place offering");
          }
        }

        // Collision
        entity.visible_position = entity.position;
        entity.visible_scale = entity.scale;
        entity.visible_rotation = entity.orientation;
      }
    }
  };
}