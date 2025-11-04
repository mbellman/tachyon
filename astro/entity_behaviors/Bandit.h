#pragma once

#include "astro/entity_behaviors/behavior.h"
#include "astro/character_dialogue.h"
#include "astro/targeting.h"
#include "astro/ui_system.h"

namespace astro {
  behavior Bandit {
    addMeshes() {
      meshes.bandit_placeholder = MODEL_MESH("./astro/3d_models/bandit/placeholder.obj", 500);
      meshes.bandit = MODEL_MESH("./astro/3d_models/guy.obj", 500);
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
      tVec3f entity_to_player = state.player_position.xz() - entity.visible_position.xz();
      float player_distance = entity_to_player.magnitude();
      auto& enemy = entity.enemy_state;

      // Player-in-range behavior
      if (player_distance < 10000.f) {
        tVec3f player_direction = entity_to_player / player_distance;
        float time_since_casting_stun = tachyon->scene.scene_time - state.spells.stun_start_time;

        // Stunned dialogue + mood changes
        {
          if (time_since_casting_stun == 0.f) {
            // Caught in the initial blast of a stun spell

            if (enemy.mood == ENEMY_IDLE) {
              // Stunned while idle
              play_random_dialogue(entity, bandit_dialogue_stunned_idle);

              // Idle -> Engaged
              // @todo ENEMY_STARTLED
              enemy.mood = ENEMY_ENGAGED;
            }
            else if (enemy.mood == ENEMY_AGITATED) {
              // Stunned while agitated
              play_random_dialogue(entity, bandit_dialogue_stunned_agitated);
            }
            else {
              // Stunned while engaged
              play_random_dialogue(entity, bandit_dialogue_stunned_engaged);

              // Engaged -> Agitated
              enemy.mood = ENEMY_AGITATED;
            }
          }
        }

        // Collision handling
        {
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
            float minimum_distance = 0.7f * (entity.visible_scale.x + bandit.visible_scale.x);

            if (distance < minimum_distance) {
              entity.visible_position = bandit.visible_position + entity_to_entity.unit() * minimum_distance;
            }
          }

          // Player collision
          // @todo factor
          float minimum_distance = 0.7f * entity.visible_scale.x + 500.f;

          if (player_distance < minimum_distance) {
            entity.visible_position = state.player_position + player_direction.invert() * minimum_distance;
          }
        }

        if (time_since_casting_stun < 3.f && enemy.mood != ENEMY_IDLE) {
          // Stunned knockback
          float knockback_factor = 3.f * time_since_casting_stun * (1.f - time_since_casting_stun);
          if (knockback_factor > 1.f) knockback_factor = 1.f;
          if (knockback_factor < 0.f) knockback_factor = 0.f;

          entity.visible_position -= player_direction * knockback_factor * 3000.f * dt;
        }
        else if (player_distance > 3000.f) {
          // Non-close-quarters behavior
          float dialogue_duration = tachyon->running_time - state.dialogue_start_time;
          tVec3f facing_direction = GetFacingDirection(entity);

          bool can_notice_player = (
            tVec3f::dot(facing_direction, player_direction) > 0.4f ||
            player_distance < 4000.f
          );

          // Chase the player when not idle
          if (enemy.mood != ENEMY_IDLE) {
            entity.visible_position += player_direction * 3000.f * dt;

            FacePlayer(entity, state);
          }

          if (enemy.mood == ENEMY_IDLE && can_notice_player) {
            // Bandit engaging the player
            enemy.mood = ENEMY_ENGAGED;

            Targeting::SetSpeakingEntity(state, entity);

            play_random_dialogue(entity, bandit_dialogue_noticed);

            // @todo alert nearby entities
          }
          else if (enemy.mood == ENEMY_ENGAGED && dialogue_duration > 5.f) {
            // Bandit behaving in an engaged state
            play_random_dialogue(entity, bandit_dialogue_engaged);
          }
          else if (enemy.mood == ENEMY_AGITATED && dialogue_duration > 5.f) {
            // Bandit behaving in an agitated state
            play_random_dialogue(entity, bandit_dialogue_agitated);
          }
        }
        else {
          // Close-quarters behavior
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
          entity.visible_position = entity.position;
          entity.visible_scale = tVec3f(0.f);
          entity.visible_rotation = entity.orientation;
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