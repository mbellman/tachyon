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

        bandit.scale = entity.scale;
        bandit.rotation = entity.orientation;
        bandit.color = entity.tint;

        // Collision
        entity.visible_scale = bandit.scale;

        // Bandit enemy AI
        // @temporary
        // @todo refactor
        if (abs(state.astro_turn_speed) > 0.f) {
          entity.visible_position = entity.position;
        } else {
          tVec3f entity_to_player = state.player_position - entity.visible_position;
          float player_distance = entity_to_player.magnitude();
          tVec3f player_direction = entity_to_player / player_distance;

          if (player_distance < 15000.f) {
            entity.visible_position += player_direction * 1000.f * 1.f / 60.f;
          }
        }

        bandit.position = entity.visible_position;

        commit(bandit);
      }
    }
  };
}