#pragma once

#include "astro/entity_behaviors/behavior.h"
#include "astro/character_dialogue.h"
#include "astro/targeting.h"
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
          play_random_dialogue(entity, {
            {
              .text = "Wha...?! I've been blinded!",
              .sound = ""
            },
            {
              .text = "I'm blinded! Where did the scoundrel go?",
              .sound = ""
            }
          });

          enemy.mood = ENEMY_AGITATED;
        }
        else if (enemy.mood == ENEMY_IDLE) {
          // Noticed
          enemy.mood = ENEMY_ENGAGED;

          Targeting::SetSpeakingEntity(state, entity);

          play_random_dialogue(entity, {
            {
              .text = "You there! Retreat, at once!",
              .sound = "./astro/audio/low_guard/retreat_at_once.mp3"
            },
            {
              .text = "Foul knave! Be on your way!",
              .sound = ""
            }
          });
        }
        else if (enemy.mood == ENEMY_ENGAGED && player_distance < 5000.f) {
          enemy.mood = ENEMY_AGITATED;

          Targeting::SetSpeakingEntity(state, entity);

          play_random_dialogue(entity, {
            {
              .text = "Cease your trespass! Or I shall strike!",
              .sound = "./astro/audio/low_guard/trespass.mp3"
            },
            {
              .text = "Stay back, or I shall arrest you!",
              .sound = ""
            }
          });
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
        auto& model = objects(meshes.low_guard)[i];

        // @todo factor
        if (IsDuringActiveTime(entity, state)) {
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

        model.position = entity.visible_position;
        model.scale = entity.visible_scale;
        model.rotation = entity.orientation;
        model.color = entity.tint;

        commit(model);
      }
    }
  };
}