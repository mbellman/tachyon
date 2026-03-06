#include "astro/combat.h"
#include "astro/entity_behaviors/behavior.h"
#include "astro/entity_manager.h"
#include "astro/player_character.h"
#include "astro/sfx.h"
#include "astro/targeting.h"

using namespace astro;

static void StunNearbyEnemies(State& state, const float scene_time) {
  for (auto& target : state.targetable_entities) {
    auto& entity = *EntityManager::FindEntity(state, target);
    auto& enemy = entity.enemy_state;
    float enemy_distance = tVec3f::distance(entity.visible_position, state.player_position);

    if (enemy_distance < 4000.f) {
      enemy.speed = -5000.f;

      enemy.last_damage_time = scene_time;
      enemy.last_attack_start_time = 0.f;
      enemy.last_attack_action_time = 0.f;
    }
  }
}

void Combat::HandleWandSwing(Tachyon* tachyon, State& state) {
  float scene_time = get_scene_time();

  // Check against nearby targets
  {
    for (auto& target : state.targetable_entities) {
      auto& entity = *EntityManager::FindEntity(state, target);
      auto& enemy = entity.enemy_state;
      float enemy_distance = tVec3f::distance(entity.visible_position, state.player_position);

      // @todo handle per enemy type (target.type)
      float attack_without_blocking_duration = 1.2f;

      if (enemy_distance < 4000.f && enemy.health > 0.f) {
        if (time_since(enemy.last_attack_start_time) > attack_without_blocking_duration) {
          // @todo factor by enemy type
          if (entity.type == LOW_GUARD) {
            // Armor blocking
            // @todo magic weapons which can pierce armor
            enemy.last_block_time = scene_time;
          }
          else if (entity.type == LESSER_GUARD) {
            // Block when facing the player
            float facing_dot = tVec3f::dot(state.player_facing_direction, GetFacingDirection(entity));

            if (facing_dot < 0.f) {
              enemy.last_block_time = scene_time;
            }
          }
        } else if (entity.type == LOW_GUARD) {
          // Armor block
          // @todo allow magical piercing attacks
          enemy.last_block_time = scene_time;
        }
      }
    }
  }

  // Distinguishing between strong and regular attacks
  {
    if (time_since(state.last_target_jump_time) < 0.5f) {
      state.last_strong_attack_time = scene_time;

      Sfx::PlaySound(SFX_WAND_STRONG_ATTACK, 0.3f);
    } else {
      Sfx::PlaySound(SFX_WAND_SWING, 0.3f);
    }
  }
}

// @todo handle recoil against solid objects
void Combat::HandleWandStrikeWindow(Tachyon* tachyon, State& state) {
  float scene_time = get_scene_time();
  float time_since_last_strong_attack = time_since(state.last_strong_attack_time);

  for (auto& target : state.targetable_entities) {
    auto& entity = *EntityManager::FindEntity(state, target);
    auto& enemy = entity.enemy_state;
    float enemy_distance = tVec3f::distance(entity.visible_position, state.player_position);

    if (enemy_distance < 3000.f) {
      SetMood(entity, ENEMY_AGITATED, scene_time);

      if (entity.type == LOW_GUARD) {
        // Wand recoil against low guards
        // @todo magic piercing weapons
        state.last_wand_swing_time = 0.f;
        state.last_wand_bounce_time = scene_time;

        // Enemy knockback
        if (time_since_last_strong_attack < 1.f) {
          enemy.speed = -5000.f;
        } else {
          enemy.speed = -3000.f;
        }

        Sfx::PlaySound(SFX_WAND_RECOIL, 0.5f);
      }
      else if (time_since(enemy.last_block_time) < 1.f) {
        // Wand recoil when the enemy is blocking
        state.last_wand_swing_time = 0.f;
        state.last_wand_bounce_time = scene_time;

        // Enemy knockback
        if (time_since_last_strong_attack < 1.f) {
          // enemy.speed = -2000.f;
          StunNearbyEnemies(state, scene_time);

          Sfx::PlaySound(SFX_WAND_ATTACK, 0.5f);
        } else {
          enemy.speed = -3000.f;

          Sfx::PlaySound(SFX_WAND_RECOIL, 0.5f);
        }
      } else if (time_since(enemy.last_damage_time) > 0.5f) {
        enemy.last_damage_time = scene_time;

        // @todo factor by enemy type
        if (entity.type == LESSER_GUARD) {
          enemy.last_attack_start_time = 0.f;
          enemy.last_attack_action_time = 0.f;

          if (time_since_last_strong_attack < 1.f) {
            // Strong attack damage + knockback
            enemy.health -= 50.f;

            StunNearbyEnemies(state, scene_time);
            // enemy.speed = -5000.f;
          } else {
            // Normal attack damage + knockback
            enemy.health -= 30.f;
            enemy.speed = -7000.f;
          }

          Sfx::PlaySound(SFX_WAND_ATTACK, 0.5f);

          if (enemy.health <= 0.f) {
            KillEnemy(entity, scene_time);

            state.has_target = false;

            Targeting::SelectNextAccessibleTarget(tachyon, state);
          }
        }
      }
    }
  }
}