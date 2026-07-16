#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior Ladder {
    addMeshes() {
      meshes.ladder_placeholder = MODEL_MESH("./astro/3d_models/ladder/placeholder.obj", 500);
      meshes.ladder_rails = MODEL_MESH("./astro/3d_models/ladder/rails.obj", 500);
      meshes.ladder_top = MODEL_MESH("./astro/3d_models/ladder/top.obj", 500);

      mesh(meshes.ladder_rails).shadow_cascade_ceiling = 3;
      mesh(meshes.ladder_top).shadow_cascade_ceiling = 3;
    }

    getMeshes() {
      return_meshes({
        meshes.ladder_rails,
        meshes.ladder_top
      });
    }

    getPlaceholderMesh() {
      return meshes.ladder_placeholder;
    }

    timeEvolve() {
      profile("  Ladder:TimeEvolve()");

      auto& meshes = state.meshes;

      reset_instances(meshes.ladder_rails);
      reset_instances(meshes.ladder_top);
      reset_instances(meshes.ladder_rung);

      for (auto& entity : state.ladders) {
        if (!IsInRangeX(entity, state, 25000.f)) continue;
        if (!IsInRangeZ(entity, state, 25000.f)) continue;
        if (!IsDuringActiveTime(entity, state)) continue;

        // Collision
        entity.visible_position = entity.position;
        entity.visible_scale = entity.scale;
        entity.visible_rotation = entity.orientation;

        // Rails
        {
          auto& rails = use_instance(meshes.ladder_rails);

          Sync(rails, entity);

          rails.color = tVec4f(1.f, 1.f, 1.f, 0.3f);
          rails.material = tVec4f(0.4f, 1.f, 0, 0);

          commit(rails);
        }

        // Top
        {
          auto& top = use_instance(meshes.ladder_top);

          Sync(top, entity);

          top.scale.y = top.scale.x;
          top.position.y = entity.position.y + entity.scale.y;

          top.color = tVec4f(1.f, 1.f, 1.f, 0.3f);
          top.material = tVec4f(0.4f, 1.f, 0, 0);

          commit(top);
        }

        // Rungs
        {
          int total_rungs = (int) ((2.f * entity.scale.y) / 600.f);
          float start_y = entity.position.y - entity.scale.y;

          for_range(1, total_rungs) {
            auto& rung = use_instance(meshes.ladder_rung);

            Sync(rung, entity);

            // Make sure the rung isn't stretched along y, if the ladder is
            rung.scale.y = rung.scale.x;

            // Place a rung every 600 y units
            rung.position.y = start_y + 600.f * float(i);

            rung.color = tVec3f(1.f);
            rung.material = tVec4f(0.4f, 1.f, 0, 0);

            commit(rung);
          }
        }
      }
    }
  };
}
