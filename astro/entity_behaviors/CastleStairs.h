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
        bool is_active_time = IsDuringActiveTime(entity, state);

        if (!is_active_time) continue;

        entity.visible_position = entity.position;
        entity.visible_rotation = entity.orientation;
        entity.visible_scale = entity.scale;

        // Platform
        if (!entity.requires_action) {
          auto& platform = use_instance(meshes.castle_stairs_platform);

          Sync(platform, entity);

          commit(platform);
        }

        // Procedural stair steps
        if (
          IsInRangeX(entity, state, 25000.f) &&
          IsInRangeZ(entity, state, 30000.f)
        ) {
          tVec3f direction = entity.orientation.toMatrix4f() * tVec3f(-1.f, 0, 0);
          tVec3f start_position = UnitEntityToWorldPosition(entity, tVec3f(1.f, 0, 0)) - direction * 200.f;
          int total_steps = (int) (2.f * entity.scale.x / 425.f);

          float base_y_scale = 200.f;

          for_range(1, total_steps) {
            auto& step = use_instance(meshes.stair_step);
            float offset_distance = (float) (i * 425);
            float progress = float(i) / float(total_steps);
            float full_y_scale = 100.f + entity.scale.y * progress * 1.15f;
            float current_y_scale = full_y_scale;

            // Only used for trigger-activated stairs
            if (entity.requires_action) {
              float trigger_time_offset = float(total_steps - (i - 1)) * 0.1f;
              float activation_alpha = time_since(entity.game_activation_time + trigger_time_offset) / 1.f;

              clamp_to_0(activation_alpha);
              clamp_to_1(activation_alpha);

              if (!entity.did_activate) {
                activation_alpha = 0.f;
              }

              current_y_scale = Tachyon_Lerpf(base_y_scale, full_y_scale, activation_alpha);

              if (i == total_steps) {
                // Make the final step full height so it covers the "back" side of the staircase
                current_y_scale = full_y_scale;
              }
            }

            Sync(step, entity);

            step.scale.x = 200.f;
            step.scale.y = current_y_scale;
            step.position = start_position + direction * offset_distance;

            commit(step);
          }
        }
      }
    }
  };
}
