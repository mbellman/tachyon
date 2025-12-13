#pragma once

#include "astro/entity_behaviors/behavior.h"
#include "astro/character_dialogue.h"
#include "astro/items.h"
#include "astro/sfx.h"
#include "astro/targeting.h"
#include "astro/ui_system.h"

namespace astro {
  behavior LowGuard {
    const static float wind_up_duration = 0.8f;
    const static float stab_duration = 0.1f;
    const static float wind_down_duration = 0.8f;
    const static float attack_duration = wind_up_duration + stab_duration + wind_down_duration;

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

      if (state.player_hp <= 0.f) {
        return;
      }

      // Combat
      if (player_distance < 10000.f) {
        tVec3f player_direction = entity_to_player / player_distance;
        float time_since_last_stun = time_since(state.spells.stun_start_time);

        bool can_notice_player = (
          tVec3f::dot(GetFacingDirection(entity), player_direction) > 0.2f ||
          (player_distance < 4000.f && state.player_velocity.magnitude() > 549.f)
        );

        // Collision handling
        // @todo factor
        {
          const float radius_factor = 1.3f;

          // @todo perform collision checks on all appropriate entity types
          // @todo perform collision checks against environment
          // @todo pathfinding
          for_entities(state.low_guards) {
            auto& guard = state.low_guards[i];

            if (IsSameEntity(entity, guard)) {
              continue;
            }

            tVec3f entity_to_entity = entity.visible_position.xz() - guard.visible_position.xz();
            float distance = entity_to_entity.magnitude();
            float minimum_distance = radius_factor * 1.5f * (entity.visible_scale.x + guard.visible_scale.x);

            if (distance < minimum_distance) {
              entity.visible_position = guard.visible_position + entity_to_entity.unit() * minimum_distance;
            }
          }

          // Player collision
          // @todo factor
          float minimum_distance = radius_factor * entity.visible_scale.x + 500.f;

          if (player_distance < minimum_distance) {
            entity.visible_position = state.player_position + player_direction.invert() * minimum_distance;
          }

          // Remain aligned with the ground
          // @todo use proper ground height
          entity.visible_position.y = 0.f;
        }

        if (time_since_last_stun >= 4.f && enemy.mood != ENEMY_IDLE) {
          FacePlayer(entity, state);

          if (enemy.mood == ENEMY_AGITATED) {
            // Chase the player
            enemy.speed += 2000.f * state.dt;
            if (enemy.speed > 3000.f) enemy.speed = 3000.f;

            bool is_attacking = time_since(enemy.last_attack_start_time) < attack_duration;

            if (is_attacking) {
              enemy.speed *= 1.f - 5.f * state.dt;
            }

            if (player_distance > 3500.f) {
              FollowPlayer(entity, player_direction, state.dt);
            }
            else if (player_distance < 2000.f) {
              AvoidPlayer(entity, player_direction, state.dt);
            }
          }
        }

        if (time_since_last_stun < 4.f) {
          // Stunned
          SetMood(entity, ENEMY_AGITATED, get_scene_time());

          play_random_dialogue(entity, low_guard_dialogue_stunned);
        }
        else if (enemy.mood == ENEMY_IDLE && can_notice_player) {
          // Noticed
          SetMood(entity, ENEMY_ENGAGED, get_scene_time());
          Targeting::SetSpeakingEntity(state, entity);

          play_random_dialogue(entity, low_guard_dialogue_engaged);
        }
        else if (
          enemy.mood == ENEMY_ENGAGED &&
          player_distance > 5000.f &&
          time_since(enemy.last_mood_change_time) < 5.f
        ) {
          // Engaged from a distance
          play_random_dialogue(entity, low_guard_dialogue_engaged);
        }
        else if (enemy.mood != ENEMY_IDLE) {
          // Agitated
          SetMood(entity, ENEMY_AGITATED, get_scene_time());
          Targeting::SetSpeakingEntity(state, entity);

          play_random_dialogue(entity, low_guard_dialogue_agitated);

          if (
            player_distance < 4000.f &&
            time_since(enemy.last_attack_start_time) > 2.f * attack_duration &&
            time_since(enemy.last_mood_change_time) > 0.5f
          ) {
            // Attacking
            enemy.last_attack_start_time = get_scene_time();
          }
        }
      } else {
        // Out of range
      }
    }

    timeEvolve() {
      profile("  LowGuard::timeEvolve()");

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
            // Astro time turning behavior
            SetMood(entity, ENEMY_IDLE, get_scene_time());

            entity.enemy_state.speed = 0.f;
            entity.visible_rotation = entity.orientation;

            if (entity.recent_positions.size() > 0) {
              // @todo factor
              // @todo only if astro turn speed < 0
              float time_since_last_reverse = time_since(entity.last_recent_position_reverse_time);

              if (time_since_last_reverse > 0.05f) {
                entity.visible_position = entity.recent_positions.back();
                entity.recent_positions.pop_back();

                entity.last_recent_position_reverse_time = get_scene_time();
              }
            }
            else if (astro_speed > 0.05f) {
              Jitter(entity, 75.f);
            }
            else {
              // Reset position/rotation when time slows down
              entity.visible_position = entity.position;
              entity.visible_rotation = entity.orientation;
            }
          } else {
            // Normal behavior

            if (entity.enemy_state.mood == ENEMY_IDLE) {
              float idle_angle = 0.2f * sinf(0.5f * get_scene_time());
              Quaternion idle_rotation = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), idle_angle);

              entity.visible_rotation = entity.orientation * idle_rotation;
            }

            // @todo factor
            {
              float time_since_last_recent_position = time_since(entity.last_recent_position_record_time);

              if (time_since_last_recent_position > 1.f && entity.enemy_state.mood != ENEMY_IDLE) {
                auto& recent_positions = entity.recent_positions;

                if (recent_positions.size() > 30) {
                  recent_positions.erase(recent_positions.begin());
                }

                recent_positions.push_back(entity.visible_position);

                entity.last_recent_position_record_time = get_scene_time();
              }
            }

            handle_enemy_behavior(LowGuard);
          }
        } else {
          // Hide and reset
          // @todo factor
          entity.visible_scale = tVec3f(0.f);
          entity.visible_position = entity.position;
          entity.visible_rotation = entity.orientation;
          entity.enemy_state.speed = 0.f;
          entity.recent_positions.clear();
        }

        // Body
        {
          auto& body = objects(meshes.low_guard_body)[i];

          body.position = entity.visible_position;
          body.scale = entity.visible_scale;
          body.rotation = entity.visible_rotation;
          body.color = entity.tint;
          body.material = tVec4f(0.5f, 1.f, 0, 0);

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

          // Idle motion
          {
            if (entity.enemy_state.mood == ENEMY_IDLE) {
              shield.position.y += 50.f * cosf(get_scene_time());
            }
          }

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

          // Idle motion
          {
            if (entity.enemy_state.mood == ENEMY_IDLE) {
              spear.position.y += 100.f * sinf(2.f * get_scene_time());
            }
          }

          // Attacking
          {
            float time_since_starting_attack = time_since(entity.enemy_state.last_attack_start_time);

            if (
              entity.enemy_state.mood == ENEMY_AGITATED &&
              time_since_starting_attack < attack_duration
            ) {
              float alpha = time_since_starting_attack / attack_duration;

              if (time_since_starting_attack < wind_up_duration) {
                // Wind-up
                float wind_up_alpha = alpha * (attack_duration / wind_up_duration);
                float angle = -0.5f * sinf(wind_up_alpha * t_HALF_PI);

                spear.rotation = spear.rotation * Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), angle);
              }
              else if (time_since_starting_attack < (wind_up_duration + stab_duration)) {
                // Stab
                float stab_alpha = Tachyon_InverseLerp(wind_up_duration, wind_up_duration + stab_duration, time_since_starting_attack);
                stab_alpha = powf(stab_alpha, 0.75f);

                Quaternion start_rotation = Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), -0.5f);
                Quaternion end_rotation = Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), t_HALF_PI);
                Quaternion current_rotation = Quaternion::slerp(start_rotation, end_rotation, stab_alpha);

                float thrust = 1000.f * sinf(stab_alpha * t_HALF_PI);
                tVec3f thrust_offset = entity.visible_rotation.toMatrix4f() * tVec3f(0, 0, 1.f) * thrust;

                spear.rotation = spear.rotation * current_rotation;
                spear.position += thrust_offset;

                tVec3f spear_tip_position = UnitObjectToWorldPosition(spear, tVec3f(0, 3.f, 0));
                float spear_tip_distance = tVec3f::distance(state.player_position, spear_tip_position);

                entity.enemy_state.last_attack_action_time = get_scene_time();

                if (
                  spear_tip_distance < 2000.f &&
                  PlayerCharacter::CanTakeDamage(tachyon, state)
                ) {
                  Sfx::PlaySound(SFX_SWORD_DAMAGE, 0.5f);
                  PlayerCharacter::TakeDamage(tachyon, state, 40.f);

                  // Knockback
                  tVec3f knockback_direction = (state.player_position - spear.position).xz().unit();

                  state.player_velocity = knockback_direction * 10000.f;
                }
              }
              else {
                // Wind-down
                float wind_down_alpha = Tachyon_InverseLerp(wind_up_duration + stab_duration, attack_duration, time_since_starting_attack);
                wind_down_alpha = Tachyon_EaseInOutf(wind_down_alpha);

                float angle = t_HALF_PI * (1.f - wind_down_alpha);
                float thrust = 1000.f * (1.f - wind_down_alpha);

                tVec3f thrust_offset = entity.visible_rotation.toMatrix4f() * tVec3f(0, 0, 1.f) * thrust;

                spear.rotation = spear.rotation * Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), angle);
                spear.position += thrust_offset;
              }
            }
          }

          commit(spear);
        }

        // Item holding
        {
          bool is_gate_key_guard = entity.item_pickup_name == "gate_key";
          bool player_has_gate_key = Items::HasItem(state, GATE_KEY);

          if (!player_has_gate_key) {
            if (is_gate_key_guard) {
              auto& gate_key = objects(meshes.item_gate_key)[0];

              if (is_active) {
                // Show the key on the active guard
                gate_key.position = UnitEntityToWorldPosition(entity, tVec3f(1.2f, 0, 0));
                gate_key.scale = tVec3f(700.f);
                gate_key.rotation = entity.visible_rotation;
                gate_key.color = tVec3f(1.f, 1.f, 0.2f);
                gate_key.material = tVec4f(0.2f, 1.f, 0, 0.4f);

                commit(gate_key);

                // Handle key retrieval
                float player_to_key_distance = tVec3f::distance(state.player_position, gate_key.position);

                if (
                  entity.enemy_state.mood == ENEMY_IDLE &&
                  player_to_key_distance < 2500.f &&
                  state.astro_turn_speed == 0.f
                ) {
                  if (did_press_key(tKey::CONTROLLER_A)) {
                    Items::CollectItem(tachyon, state, GATE_KEY);

                    remove_object(gate_key);
                  } else {
                    // @todo only if not noticed by any guards!
                    // @bug showing this dialogue resets the current dialogue,
                    // allowing the adjacent guard to repeatedly restart his
                    // "cease your trespass!" line and spam the audio line
                    UISystem::ShowTransientDialogue(tachyon, state, "[X] Collect gate key");

                    gate_key.color.rgba |= 0x000A;

                    commit(gate_key);
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