#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior Bandit {
    addMeshes() {
      meshes.bandit_placeholder = CUBE_MESH(500);
      meshes.bandit = CUBE_MESH(500);
    }

    getMeshes() {
      return_meshes({
        meshes.bandit
      });
    }

    getPlaceholderMesh() {
      return meshes.bandit_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      // @todo @optimize only iterate over on-screen/in-range entities
      // once that list is built
      for_entities(state.bandits) {
        auto& entity = state.bandits[i];
        auto& bandit = objects(meshes.bandit)[i];

        bool active = (
          state.astro_time >= entity.astro_start_time &&
          state.astro_time <= entity.astro_end_time
        );

        if (active) {
          entity.visible_scale = entity.scale;

          float astro_speed = abs(state.astro_turn_speed);

          if (astro_speed > 0.f) {
            if (astro_speed < 0.05f) {
              // Do nothing
            }
            else if (
              state.astro_time - entity.astro_start_time < 5.f ||
              entity.astro_end_time - state.astro_time < 5.f
            ) {
              // Jitter the visible position as turn speed picks up to suggest movement
              // @todo factor
              entity.visible_position.x += Tachyon_GetRandom(-200.f, 200.f);
              entity.visible_position.z += Tachyon_GetRandom(-200.f, 200.f);
            }
            else if (astro_speed < 0.2f && entity.visible_position != entity.position) {
              // Jitter the visible position as turn speed picks up to suggest movement
              // @todo factor
              entity.visible_position.x += Tachyon_GetRandom(-50.f, 50.f);
              entity.visible_position.z += Tachyon_GetRandom(-50.f, 50.f);
            }
            else {
              // Reset
              // @todo factor
              entity.visible_position = entity.position;
            }
          } else {
            // Bandit enemy AI
            // @temporary
            // @todo refactor
            tVec3f entity_to_player = state.player_position - entity.visible_position;
            float player_distance = entity_to_player.magnitude();
            tVec3f player_direction = entity_to_player / player_distance;

            if (player_distance < 10000.f) {
              if (player_distance > 3000.f) {
                float time_since_last_stun = tachyon->running_time - state.spells.stun_start_time;

                if (time_since_last_stun < 4.f) {
                  entity.visible_position -= player_direction * 500.f * dt;
                } else {
                  entity.visible_position += player_direction * 3000.f * dt;
                }
              } else {
                // @todo strafe around the player
              }
            }
          }
        } else {
          // Hide and reset
          // @todo factor
          entity.visible_scale = tVec3f(0.f);
          entity.visible_position = entity.position;
        }

        bandit.position = entity.visible_position;
        bandit.scale = entity.visible_scale;
        bandit.rotation = entity.orientation;
        bandit.color = entity.tint;

        commit(bandit);
      }
    }
  };
}