#pragma once

#include "astro/entity_behaviors/behavior.h"
#include "astro/character_dialogue.h"
#include "astro/items.h"
#include "astro/sfx.h"
#include "astro/targeting.h"
#include "astro/ui_system.h"

namespace astro {
  behavior LesserGuard {
    const static float wind_up_duration = 1.f;
    const static float stab_duration = 0.1f;
    const static float wind_down_duration = 0.8f;
    const static float attack_duration = wind_up_duration + stab_duration + wind_down_duration;

    addMeshes() {
      meshes.lesser_guard_placeholder = MODEL_MESH("./astro/3d_models/guy.obj", 500);
      meshes.lesser_guard_body = MODEL_MESH("./astro/3d_models/low_guard/body.obj", 500);
      meshes.lesser_guard_shield = MODEL_MESH("./astro/3d_models/low_guard/shield.obj", 500);
      meshes.lesser_guard_sword = MODEL_MESH("./astro/3d_models/lesser_guard/sword.obj", 500);
    }

    getMeshes() {
      return_meshes({
        meshes.lesser_guard_body,
        meshes.lesser_guard_shield,
        meshes.lesser_guard_sword
      });
    }

    getPlaceholderMesh() {
      return meshes.lesser_guard_placeholder;
    }

    handleEnemyBehavior() {
      tVec3f entity_to_player = state.player_position - entity.visible_position;
      float player_distance = entity_to_player.magnitude();
      auto& enemy = entity.enemy_state;

      if (state.player_hp <= 0.f) {
        return;
      }

      // Idle animation
      // @temporary
      if (entity.enemy_state.mood == ENEMY_IDLE) {
        float idle_angle = 0.2f * sinf(0.5f * get_scene_time());
        Quaternion idle_rotation = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), idle_angle);

        entity.visible_rotation = entity.orientation * idle_rotation;
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
        {
          // @todo perform collision checks on all appropriate entity types
          // @todo perform collision checks against environment
          // @todo pathfinding
          PreventEntityCollisions(entity, state.lesser_guards, 1.95f);

          // Player collision
          PreventEntityPlayerCollision(entity, state.player_position, player_direction, player_distance, 1.3f);
        }

        if (time_since_last_stun >= 4.f && enemy.mood != ENEMY_IDLE) {
          FacePlayer(entity, state);

          if (enemy.mood == ENEMY_AGITATED) {
            // Gain speed toward the player
            enemy.speed += 2000.f * state.dt;
            if (enemy.speed > 3000.f) enemy.speed = 3000.f;

            // Slow down when attacking
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
      profile("  LesserGuard::timeEvolve()");

      auto& meshes = state.meshes;

      // @todo @optimize only iterate over on-screen/in-range entities
      for_entities(state.lesser_guards) {
        auto& entity = state.lesser_guards[i];
        bool is_active = IsDuringActiveTime(entity, state);

        if (is_active) {
          entity.visible_scale = entity.scale;

          float astro_speed = abs(state.astro_turn_speed);

          if (astro_speed > 0.f) {
            // Astro time turning behavior
            SoftResetEntity(entity, get_scene_time());

            if (entity.recent_positions.size() > 0) {
              // @todo only if astro turn speed < 0
              ReloadRecentPosition(entity, get_scene_time());
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

            TrackRecentPositions(entity, get_scene_time());

            if (entity.enemy_state.health > 0.f) {
              handle_enemy_behavior(LesserGuard);
            }

            // Remain aligned with the ground
            // @todo use proper ground height
            entity.visible_position.y = 0.f;
          }
        } else {
          HardResetEntity(entity);
        }

        float death_alpha = 0.f;

        if (entity.enemy_state.last_death_time != 0.f) {
          death_alpha = 2.f * time_since(entity.enemy_state.last_death_time);
          if (death_alpha > 1.f) death_alpha = 1.f;
        }

        // Body
        {
          auto& body = objects(meshes.lesser_guard_body)[i];

          body.position = entity.visible_position;
          body.scale = entity.visible_scale;
          body.rotation = entity.visible_rotation;
          // body.color = entity.tint;
          body.color = tVec3f(0.8f, 0.4f, 0.2f);
          body.material = tVec4f(0.6f, 0, 0, 0.4f);

          if (death_alpha > 0.f) {
            Quaternion death_rotation = entity.visible_rotation * (
              Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), -t_HALF_PI)
            );

            body.rotation = Quaternion::slerp(body.rotation, death_rotation, death_alpha);
            body.position.y = Tachyon_Lerpf(body.position.y, -1100.f, death_alpha);
          }

          commit(body);
        }

        // Shield
        {
          auto& shield = objects(meshes.lesser_guard_shield)[i];

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

          // Blocking
          {
            float time_since_blocking = time_since(entity.enemy_state.last_block_time);

            if (time_since_blocking < 1.f) {
              float alpha = time_since_blocking;
              if (alpha > 1.f) alpha = 1.f;

              tVec3f enemy_direction = GetFacingDirection(entity);
              float offset_factor = powf(sinf(alpha * t_PI), 2.f);

              // Forward motion
              shield.position += enemy_direction * 200.f * offset_factor;
              // Upward motion
              shield.position.y += 600.f * offset_factor;
            }
          }

          commit(shield);
        }

        // Sword
        {
          auto& sword = objects(meshes.lesser_guard_sword)[i];

          sword.position = UnitEntityToWorldPosition(entity, tVec3f(-1.f, 1.f, 1.2f));
          sword.scale = entity.visible_scale * tVec3f(1.f, 0.4f, 1.f) * 1.25f; // @temporary
          sword.rotation = entity.visible_rotation;
          sword.color = tVec3f(0.6f);
          sword.material = tVec4f(0.2f, 1.f, 0, 0);

          // Idle motion
          {
            if (entity.enemy_state.mood == ENEMY_IDLE) {
              sword.position.y += 100.f * sinf(2.f * get_scene_time());
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

              tVec3f enemy_direction = GetFacingDirection(entity);
              tVec3f enemy_right = tVec3f::cross(enemy_direction, tVec3f(0, 1.f, 0));

              // Animation steps
              AnimationStep s1;
              s1.duration = wind_up_duration;
              s1.offset = tVec3f(0.f);
              s1.rotation = Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), 0.f);

              AnimationStep s2;
              s2.duration = stab_duration;
              s2.offset = enemy_right * 500.f - enemy_direction * 500.f;
              s2.rotation = (
                Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), -0.7f) *
                Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), 1.5f)
              );

              AnimationStep s3;
              s3.duration = wind_down_duration;
              s3.offset = tVec3f(0, -1000.f, 0) + enemy_direction * 1000.f - enemy_right * 1000.f;
              s3.rotation = (
                Quaternion::fromAxisAngle(tVec3f(0, 0, 1.f), 2.f) *
                Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), 2.1f)
              );

              AnimationSequence attack_animation;
              attack_animation.steps = { s1, s2, s3, s1 };

              // Sample the animation
              TransformState sample = SimpleAnimation::Sample(attack_animation, time_since_starting_attack);
              sword.position += sample.offset;
              sword.rotation = entity.visible_rotation * sample.rotation;

              // Player hit detection
              if (
                time_since_starting_attack > wind_up_duration &&
                time_since_starting_attack < (wind_up_duration + stab_duration)
              ) {
                tVec3f sword_tip_position = UnitObjectToWorldPosition(sword, tVec3f(0, 2.5f, 0));
                float sword_tip_distance = tVec3f::distance(state.player_position, sword_tip_position);

                entity.enemy_state.last_attack_action_time = get_scene_time();

                if (
                  sword_tip_distance < 2000.f &&
                  PlayerCharacter::CanTakeDamage(tachyon, state)
                ) {
                  Sfx::PlaySound(SFX_SWORD_DAMAGE, 0.5f);
                  PlayerCharacter::TakeDamage(tachyon, state, 40.f);

                  // Knockback
                  tVec3f knockback_direction = (state.player_position - sword.position).xz().unit();

                  state.player_velocity = knockback_direction * 10000.f;
                }
              }
            }
          }

          commit(sword);
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