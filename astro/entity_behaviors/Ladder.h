#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior Ladder {
    addMeshes() {
      meshes.ladder_placeholder = MODEL_MESH("./astro/3d_models/ladder/placeholder.obj", 500);
      meshes.ladder_rails = MODEL_MESH("./astro/3d_models/ladder/rails.obj", 500);
    }

    getMeshes() {
      return_meshes({
        meshes.ladder_rails
      });
    }

    getPlaceholderMesh() {
      return meshes.ladder_placeholder;
    }

    timeEvolve() {
      profile("Ladder:TimeEvolve()");

      auto& meshes = state.meshes;

      reset_instances(meshes.ladder_rails);
      reset_instances(meshes.ladder_rung);

      for (auto& entity : state.ladders) {
        if (!IsInRangeX(entity, state, 20000.f)) continue;
        if (!IsInRangeZ(entity, state, 20000.f)) continue;

        // Collision
        entity.visible_position = entity.position;
        entity.visible_scale = entity.scale;
        entity.visible_rotation = entity.orientation;

        // Ladder
        {
          auto& ladder = use_instance(meshes.ladder_rails);

          Sync(ladder, entity);

          commit(ladder);
        }

        // Rungs
        {
          int total_rungs = (int) (2.f * entity.scale.y) / 600.f;
          float start_y = entity.position.y - entity.scale.y;

          for_range(1, total_rungs) {
            auto& rung = use_instance(meshes.ladder_rung);

            Sync(rung, entity);

            // Make sure the rung isn't stretched along y, if the ladder is
            rung.scale.y = rung.scale.x;

            // Place a rung every 600 y units
            rung.position.y = start_y + 600.f * float(i);

            commit(rung);
          }
        }
      }
    }
  };
}
