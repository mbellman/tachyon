#pragma once

#include "astro/entity_behaviors/behavior.h"
#include "astro/targeting.h"
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

        if (time_since_casting_stun < 0.25f) {
          // Caught in the initial blast of a stun spell
          if (enemy.mood == ENEMY_AGITATED) {
            play_random_dialogue(entity, {
              {
                .text = "Agh! You filthy coward!",
                .sound = ""
              },
              {
                .text = "Wretched little bastard!",
                .sound = ""
              }
            });
          } else {
            play_random_dialogue(entity, {
              {
                .text = "Argh! The bastard blinded me!",
                .sound = "./astro/audio/bandit/blinded.mp3"
              }
            });
          }
        }
        else if (time_since_casting_stun < 0.5f) {
          // Agitate stunned enemies after a delay to allow
          // distinct engaged + already-agitated dialogue lines
          enemy.mood = ENEMY_AGITATED;
        }

        if (time_since_casting_stun < 3.f) {
          // Stunned
          float knockback_factor = 3.f * time_since_casting_stun * (1.f - time_since_casting_stun);
          if (knockback_factor > 1.f) knockback_factor = 1.f;
          if (knockback_factor < 0.f) knockback_factor = 0.f;

          entity.visible_position -= player_direction * knockback_factor * 3000.f * dt;
        }
        else if (player_distance > 3000.f) {
          // Non-strafing combat
          float time_since_casting_stun = tachyon->running_time - state.spells.stun_start_time;
          float dialogue_duration = tachyon->running_time - state.dialogue_start_time;

          // Chase the player
          entity.visible_position += player_direction * 3000.f * dt;

          FacePlayer(entity, state);

          // @todo perform collision checks on all appropriate entity types
          // @todo perform collision checks against environment
          // @todo pathfinding
          for_entities(state.bandits) {
            auto& bandit = state.bandits[i];

            if (IsSameEntity(entity, bandit)) {
              continue;
            }

            tVec3f entity_to_entity = entity.visible_position.xz() - bandit.visible_position.xz();
            float distance = entity_to_entity.magnitude();
            float minimum_distance = 1.6f * (entity.visible_scale.x + bandit.visible_scale.x);

            if (distance < minimum_distance) {
              entity.visible_position = bandit.visible_position + entity_to_entity.unit() * minimum_distance;
            }
          }

          if (enemy.mood == ENEMY_IDLE) {
            enemy.mood = ENEMY_ENGAGED;

            Targeting::SetSpeakingEntity(state, entity);

            play_random_dialogue(entity, {
              {
                .text = "Look, we've got one!",
                .sound = "./astro/audio/bandit/got_one.mp3"
              },
              {
                .text = "All by our lonesome, are we?",
                .sound = "./astro/audio/bandit/lonesome.mp3"
              }
            });
          }
          else if (enemy.mood == ENEMY_ENGAGED && dialogue_duration > 5.f) {
            play_random_dialogue(entity, {
              {
                .text = "I'll make quick work of him!",
                .sound = ""
              },
              {
                .text = "Let's not make this difficult!",
                .sound = ""
              },
              {
                .text = "Oi, where do you think you're going?",
                .sound = ""
              }
            });
          }
          else if (enemy.mood == ENEMY_AGITATED && dialogue_duration > 5.f) {
            play_random_dialogue(entity, {
              {
                .text = "Dirty rat! You're in for it now!",
                .sound = ""
              },
              {
                .text = "Dirty rat! You're in for it now!",
                .sound = ""
              },
              {
                .text = "Now you've asked for it!",
                .sound = ""
              }
            });
          }
        }
        else {
          // Strafing combat
          // @todo strafe around the player
          FacePlayer(entity, state);
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
        auto& model = objects(meshes.bandit)[i];

        // @todo factor
        if (IsDuringActiveTime(entity, state)) {
          entity.visible_scale = entity.scale;

          float astro_speed = abs(state.astro_turn_speed);

          if (astro_speed > 0.f) {
            entity.enemy_state.mood = ENEMY_IDLE;
            entity.visible_rotation = entity.orientation;

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

        model.position = entity.visible_position;
        model.scale = entity.visible_scale;
        model.rotation = entity.visible_rotation;
        model.color = entity.tint;

        commit(model);
      }
    }
  };
}