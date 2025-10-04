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
          state.astro_time > entity.astro_start_time &&
          state.astro_time < entity.astro_end_time
        );

        if (active) {
          entity.visible_scale = entity.scale;

          // Bandit movement AI
          // @temporary
          // @todo refactor
          float astro_speed = abs(state.astro_turn_speed);

          if (astro_speed > 0.f) {
            if (astro_speed < 0.1f) {
              // Do nothing
            }
            else if (astro_speed < 0.2f && entity.visible_position != entity.position) {
              // Jitter the visible position as turn speed picks up to suggest movement
              entity.visible_position.x += Tachyon_GetRandom(-400.f, 400.f);
              entity.visible_position.z += Tachyon_GetRandom(-400.f, 400.f);
            }
            else {
              // Once we're turning fast enough, reset the position completely
              entity.visible_position = entity.position;
            }
          } else {
            tVec3f entity_to_player = state.player_position - entity.visible_position;
            float player_distance = entity_to_player.magnitude();
            tVec3f player_direction = entity_to_player / player_distance;

            if (player_distance < 12000.f) {
              entity.visible_position += player_direction * 1000.f * 1.f / 60.f;
            }
          }
        } else {
          // Hide and do nothing while not within active time range
          entity.visible_scale = tVec3f(0.f);
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