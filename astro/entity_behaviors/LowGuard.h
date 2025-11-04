#pragma once

#include "astro/entity_behaviors/behavior.h"
#include "astro/character_dialogue.h"
#include "astro/targeting.h"
#include "astro/ui_system.h"

namespace astro {
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
      meshes.low_guard_spear = MODEL_MESH("./astro/3d_models/low_guard/spear.obj", 500);
    }

    getMeshes() {
      return_meshes({
        meshes.low_guard_body,
        meshes.low_guard_shield,
        meshes.low_guard_spear
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

        bool can_notice_player = (
          tVec3f::dot(GetFacingDirection(entity), player_direction) > 0.2f ||
          (player_distance < 4000.f && state.player_velocity.magnitude() > 600.f)
        );

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
        else if (enemy.mood == ENEMY_IDLE && can_notice_player) {
          // Noticed
          enemy.mood = ENEMY_ENGAGED;

          Targeting::SetSpeakingEntity(state, entity);

          play_random_dialogue(entity, low_guard_dialogue_engaged);
        }
        else if (enemy.mood == ENEMY_ENGAGED && player_distance > 5000.f) {
          // Engaged from a distance
          play_random_dialogue(entity, low_guard_dialogue_engaged);
        }
        else if (enemy.mood != ENEMY_IDLE) {
          // Agitated, up close
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
        bool is_active = IsDuringActiveTime(entity, state);

        // @todo factor
        if (is_active) {
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

        // Body
        {
          auto& body = objects(meshes.low_guard_body)[i];

          body.position = entity.visible_position;
          body.scale = entity.visible_scale;
          body.rotation = entity.visible_rotation;
          body.color = entity.tint;

          commit(body);
        }

        // Shield
        {
          auto& shield = objects(meshes.low_guard_shield)[i];

          shield.position = UnitEntityToWorldPosition(entity, tVec3f(1.f, 0.2f, 1.2f));
          shield.scale = entity.visible_scale * tVec3f(1.f, 0.4f, 1.f); // @temporary
          shield.rotation = entity.visible_rotation;
          shield.color = tVec3f(0.4f);
          shield.material = tVec4f(0.2f, 1.f, 0, 0);

          commit(shield);
        }

        // Spear
        {
          auto& spear = objects(meshes.low_guard_spear)[i];

          spear.position = UnitEntityToWorldPosition(entity, tVec3f(-1.f, 0.f, 1.2f));
          spear.scale = entity.visible_scale * tVec3f(1.f, 0.4f, 1.f) * 1.25f; // @temporary
          spear.rotation = entity.visible_rotation;
          spear.color = tVec3f(0.6f);
          spear.material = tVec4f(0.2f, 1.f, 0, 0);

          commit(spear);
        }

        // Item holding
        {
          // @temporary
          // @todo allow entities to have configurable held items
          bool is_gate_key_guard = tVec3f::distance(entity.position, tVec3f(84410.f, 46.f, -8000.f)) < 1000.f;

          bool player_has_gate_key = Items::HasItem(state, GATE_KEY);

          if (!player_has_gate_key) {
            if (is_gate_key_guard) {
              auto& gate_key = objects(meshes.item_gate_key)[0];

              if (is_active) {
                // Show the key on the active guard
                gate_key.position = UnitEntityToWorldPosition(entity, tVec3f(1.2f, 0, 0));
                gate_key.scale = is_active ? tVec3f(600.f) : tVec3f(0.f);
                gate_key.rotation = entity.visible_rotation;
                gate_key.color = tVec3f(1.f, 1.f, 0.2f);
                gate_key.material = tVec4f(0.2f, 1.f, 0, 0);

                commit(gate_key);

                // Handle key retrieval
                float player_to_key_distance = tVec3f::distance(state.player_position, gate_key.position);

                if (entity.enemy_state.mood == ENEMY_IDLE && player_to_key_distance < 2000.f) {
                  if (did_press_key(tKey::CONTROLLER_A)) {
                    Items::CollectItem(tachyon, state, GATE_KEY);

                    remove_object(gate_key);
                  } else {
                    UISystem::ShowDialogue(tachyon, state, "[X] Collect gate key");
                  }
                }
              } else {
                // Hide the key when the guard is not active
                gate_key.scale = tVec3f(0.f);

                commit(gate_key);
              }
            }
          }
        }
      }
    }
  };
}