#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior CastleStairs {
    addMeshes() {
      meshes.castle_stairs_placeholder = MODEL_MESH("./astro/3d_models/castle_stairs/placeholder.obj", 500);
      meshes.castle_stairs_platform = MODEL_MESH("./astro/3d_models/castle_stairs/platform.obj", 500);
      // @todo
      meshes.castle_stairs_step = CUBE_MESH(500);
    }

    getMeshes() {
      return_meshes({
        meshes.castle_stairs_platform
      });
    }

    getPlaceholderMesh() {
      return meshes.castle_stairs_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      reset_instances(meshes.castle_stairs_platform);
      reset_instances(meshes.stair_step);

      for (auto& entity : state.castle_stairs) {
        if (!IsDuringActiveTime(entity, state)) continue;

        entity.visible_position = entity.position;
        entity.visible_rotation = entity.orientation;
        entity.visible_scale = entity.scale;

        // Platform
        {
          auto& platform = use_instance(meshes.castle_stairs_platform);

          Sync(platform, entity);

          commit(platform);
        }

        // Procedural stair steps
        if (
          IsInRangeX(entity, state, 25000.f) &&
          IsInRangeZ(entity, state, 25000.f)
        ) {
          tVec3f direction = entity.orientation.toMatrix4f() * tVec3f(-1.f, 0, 0);
          tVec3f start_position = UnitEntityToWorldPosition(entity, tVec3f(1.f, 0, 0)) - direction * 200.f;
          int total_steps = (int) (2.f * entity.scale.x / 425.f);

          for_range(1, total_steps) {
            auto& step = use_instance(meshes.stair_step);
            float offset_distance = (float) (i * 425);
            float progress = float(i) / float(total_steps);

            Sync(step, entity);

            step.scale.x = 200.f;
            step.scale.y = 100.f + entity.scale.y * progress * 1.15f;
            step.position = start_position + direction * offset_distance;

            commit(step);
          }
        }
      }
    }
  };
}
