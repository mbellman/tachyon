#pragma once

#include "astro/entity_behaviors/behavior.h"
#include "astro/ui_system.h"

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

    handleEnemyBehavior() {
      tVec3f entity_to_player = state.player_position - entity.visible_position;
      float player_distance = entity_to_player.magnitude();
      auto& enemy = entity.enemy_state;

      // Combat
      if (player_distance < 10000.f) {
        tVec3f player_direction = entity_to_player / player_distance;
        float time_since_last_stun = tachyon->running_time - state.spells.stun_start_time;

        if (time_since_last_stun < 4.f) {
          // Stunned
          show_random_dialogue({
            "Wha...?! I've been blinded!",
            "What was that light? I cannot see!"
          });

          enemy.mood = ENEMY_AGITATED;
        }
        else if (enemy.mood == ENEMY_IDLE) {
          // Noticed
          show_random_dialogue({
            "You there! Retreat, at once!",
            "Knave! Be on your way!"
          });

          enemy.mood = ENEMY_ENGAGED;
        }
        else if (enemy.mood == ENEMY_ENGAGED && player_distance < 5000.f) {
          show_random_dialogue({
            "Cease your trespass! Or I shall strike!",
            "Stay back, or I shall arrest you!"
          });

          enemy.mood = ENEMY_AGITATED;
        }
      } else {
        // Out of range
        enemy.mood = ENEMY_IDLE;
      }
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

        // @todo factor
        if (active) {
          entity.visible_scale = entity.scale;

          float astro_speed = abs(state.astro_turn_speed);

          if (astro_speed > 0.f) {
            entity.enemy_state.mood = ENEMY_IDLE;

            if (astro_speed < 0.05f) {
              // Do nothing
            }
            else if (
              state.astro_time - entity.astro_start_time < 5.f ||
              entity.astro_end_time - state.astro_time < 5.f
            ) {
              Jitter(entity, 200.f);
            }
            else if (astro_speed < 0.2f && entity.visible_position != entity.position) {
              Jitter(entity, 50.f);
            }
            else {
              // Reset
              // @todo factor
              entity.visible_position = entity.position;
            }
          } else {
            handle_enemy_behavior(LowGuard);
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