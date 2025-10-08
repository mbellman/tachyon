#pragma once

#include "astro/entity_behaviors/behavior.h"
#include "astro/ui_system.h"

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

    handleEnemyBehavior() {
      tVec3f entity_to_player = state.player_position - entity.visible_position;
      float player_distance = entity_to_player.magnitude();
      auto& enemy = entity.enemy_state;

      // Combat
      if (player_distance < 10000.f) {
        tVec3f player_direction = entity_to_player / player_distance;
        float time_since_casting_stun = tachyon->running_time - state.spells.stun_start_time;

        if (time_since_casting_stun < 4.f) {
          // Stunned
          float knockback_factor = 3.f * time_since_casting_stun * (1.f - time_since_casting_stun);
          if (knockback_factor > 1.f) knockback_factor = 1.f;
          if (knockback_factor < 0.f) knockback_factor = 0.f;

          entity.visible_position -= player_direction * knockback_factor * 3000.f * dt;

          UISystem::ShowDialogue(tachyon, state, "Argh! The bastard blinded me!");

          enemy.mood = ENEMY_AGITATED;
        }
        else if (player_distance > 3000.f) {
          // Non-strafing combat
          float time_since_casting_stun = tachyon->running_time - state.spells.stun_start_time;
          float targeting_duration = tachyon->running_time - state.target_start_time;

          // Chase the player
          entity.visible_position += player_direction * 3000.f * dt;

          if (enemy.mood == ENEMY_AGITATED) {
            show_random_dialogue({
              "Scoundrel! You're in for it now!",
              "Oh, you're dead!"
            });
          }
          else if (enemy.mood == ENEMY_IDLE) {
            show_random_dialogue({
              "Look, we've got one!",
              "He looks good for the taking!"
            });

            enemy.mood = ENEMY_ENGAGED;
          }
          else if (enemy.mood == ENEMY_ENGAGED && targeting_duration > 5.f) {
            show_random_dialogue({
              "I'll make quick work of this one!",
              "No need to make things difficult!"
            });
          }
        }
        else {
          // Strafing combat
          // @todo strafe around the player
        }
      } else {
        // Out of range
      }
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
            handle_enemy_behavior(Bandit);
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