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
        float time_since_last_stun = tachyon->scene.scene_time - state.spells.stun_start_time;

        if (time_since_last_stun >= 4.f && enemy.mood != ENEMY_IDLE) {
          FacePlayer(entity, state);

          // @todo collision handling

          if (enemy.mood == ENEMY_AGITATED) {
            // @todo FollowPlayer()
            float speed = player_distance * 0.5f;

            entity.visible_position += entity_to_player.unit() * speed * dt;
          }
        }

        if (time_since_last_stun < 4.f) {
          // Stunned
          play_random_dialogue(entity, low_guard_dialogue_stunned);

          enemy.mood = ENEMY_AGITATED;
        }
        else if (enemy.mood == ENEMY_IDLE) {
          // Noticed
          // @todo use line of sight to player
          enemy.mood = ENEMY_ENGAGED;

          Targeting::SetSpeakingEntity(state, entity);

          play_random_dialogue(entity, low_guard_dialogue_engaged);
        }
        else if (player_distance > 5000.f) {
          play_random_dialogue(entity, low_guard_dialogue_engaged);
        }
        else {
          enemy.mood = ENEMY_AGITATED;

          Targeting::SetSpeakingEntity(state, entity);

          play_random_dialogue(entity, low_guard_dialogue_agitated);
        }
      } else {
        // Out of range
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
            handle_enemy_behavior(LowGuard);
          }
        } else {
          // Hide and reset
          // @todo factor
          entity.visible_scale = tVec3f(0.f);
          entity.visible_position = entity.position;
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