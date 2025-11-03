#pragma once

#include "astro/entity_behaviors/behavior.h"
#include "astro/character_dialogue.h"
#include "astro/targeting.h"
#include "astro/ui_system.h"

namespace astro {
  // @todo factor
  // @todo rename this
  static tVec3f UnitEntityToWorld(const GameEntity& entity, const tVec3f& position) {
    tVec3f translation = entity.visible_position;
    Quaternion rotation = entity.visible_rotation;
    tVec3f scale = entity.visible_scale;

    return translation + rotation.toMatrix4f() * (position * scale);
  }

  // @todo factor
  static float InverseLerp(const float start, const float end, const float value) {
    float alpha = (value - start) / (end - start);
    if (alpha < 0.f) alpha = 0.f;
    if (alpha > 1.f) alpha = 1.f;

    return alpha;
  }

  behavior LowGuard {
    addMeshes() {
      meshes.low_guard_placeholder = MODEL_MESH("./astro/3d_models/guy.obj", 500);
      meshes.low_guard_body = MODEL_MESH("./astro/3d_models/low_guard/body.obj", 500);
      meshes.low_guard_shield = MODEL_MESH("./astro/3d_models/low_guard/shield.obj", 500);
    }

    getMeshes() {
      return_meshes({
        meshes.low_guard_body,
        meshes.low_guard_shield
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
            float speed;

            if (player_distance < 8000.f) {
              const float faster_speed = 3000.f;
              const float slower_speed = 2000.f;
              float alpha = InverseLerp(8000.f, 3000.f, player_distance);

              speed = Tachyon_Lerpf(faster_speed, slower_speed, alpha);
            } else {
              const float slower_speed = 1000.f;
              const float faster_speed = 3000.f;
              float alpha = InverseLerp(10000.f, 8000.f, player_distance);

              speed = Tachyon_Lerpf(slower_speed, faster_speed, alpha);
            }

            // @todo FollowPlayer()
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
        else if (enemy.mood == ENEMY_ENGAGED && player_distance > 5000.f) {
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
      for_entities(state.low_guards) {
        auto& entity = state.low_guards[i];

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

        auto& body = objects(meshes.low_guard_body)[i];
        auto& shield = objects(meshes.low_guard_shield)[i];

        body.position = entity.visible_position;
        body.scale = entity.visible_scale;
        body.rotation = entity.visible_rotation;
        body.color = entity.tint;

        shield.position = UnitEntityToWorld(entity, tVec3f(1.f, 0.2f, 1.2f));
        shield.scale = entity.visible_scale * tVec3f(1.f, 0.4f, 1.f); // @temporary
        shield.rotation = entity.visible_rotation;
        shield.color = tVec3f(0.4f);
        shield.material = tVec4f(0.2f, 1.f, 0, 0);

        commit(body);
        commit(shield);
      }
    }
  };
}