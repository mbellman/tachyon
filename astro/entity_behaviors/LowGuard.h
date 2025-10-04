#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior LowGuard {
    addMeshes() {
      meshes.low_guard_placeholder = CUBE_MESH(500);
      meshes.low_guard = CUBE_MESH(500);
    }

    getMeshes() {
      return_meshes({
        meshes.low_guard
      });
    }

    getPlaceholderMesh() {
      return meshes.low_guard_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      // @todo @optimize only iterate over on-screen/in-range entities
      // once that list is built
      for_entities(state.low_guards) {
        auto& entity = state.low_guards[i];
        auto& guard = objects(meshes.low_guard)[i];

        bool active = (
          state.astro_time >= entity.astro_start_time &&
          state.astro_time <= entity.astro_end_time
        );

        if (active) {
          entity.visible_scale = entity.scale;

          float astro_speed = abs(state.astro_turn_speed);

          if (astro_speed > 0.f) {
            if (astro_speed < 0.1f) {
              // Reset
              // @todo factor
              entity.visible_position = entity.position;
            }
            else if (astro_speed < 0.2f && entity.visible_position != entity.position) {
              // Jitter the visible position as turn speed picks up to suggest movement
              // @todo factor
              entity.visible_position.x += Tachyon_GetRandom(-400.f, 400.f);
              entity.visible_position.z += Tachyon_GetRandom(-400.f, 400.f);
            }
            else if (state.astro_time - entity.astro_start_time < 5.f) {
              // Jitter the visible position as turn speed picks up to suggest movement
              // @todo factor
              entity.visible_position.x += Tachyon_GetRandom(-200.f, 200.f);
              entity.visible_position.z += Tachyon_GetRandom(-200.f, 200.f);
            }
            else {
              // Reset
              // @todo factor
              entity.visible_position = entity.position;
            }
          } else {
            // @todo Low Guard enemy AI
          }
        } else {
          // Hide and reset
          // @todo factor
          entity.visible_scale = tVec3f(0.f);
          entity.visible_position = entity.position;
        }

        guard.position = entity.visible_position;
        guard.scale = entity.visible_scale;
        guard.rotation = entity.orientation;
        guard.color = entity.tint;

        commit(guard);
      }
    }
  };
}