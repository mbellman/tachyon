#include "astro/combat.h"
#include "astro/entity_behaviors/behavior.h"
#include "astro/entity_manager.h"
#include "astro/player_character.h"
#include "astro/sfx.h"
#include "astro/targeting.h"

using namespace astro;

static void BreakEnemy(GameEntity& entity, const float scene_time) {
  auto& enemy = entity.enemy_state;

  enemy.speed = -5000.f;
  enemy.last_break_time = scene_time;
  enemy.last_attack_start_time = 0.f;
  enemy.last_attack_action_time = 0.f;
  enemy.last_block_time = 0.f;
}

static void StunEnemy(GameEntity& entity, const float scene_time) {
  auto& enemy = entity.enemy_state;

  enemy.speed = -4000.f;
  enemy.last_damage_time = scene_time;
  enemy.last_attack_start_time = 0.f;
  enemy.last_attack_action_time = 0.f;
  enemy.last_block_time = 0.f;
}

static void StunNearbyEnemies(State& state, const float scene_time) {
  for (auto& target : state.targetable_entities) {
    auto& entity = *EntityManager::FindEntity(state, target);
    auto& enemy = entity.enemy_state;
    float enemy_distance = tVec3f::distance(entity.visible_position, state.player_position);
    bool is_active_target = state.has_target && IsSameEntity(entity, state.target_entity);

    if (enemy_distance < 4000.f && !is_active_target) {
      StunEnemy(entity, scene_time);
    }
  }
}

static void HandleLowGuardWandStrike(Tachyon* tachyon, State& state, GameEntity& entity) {
  auto& enemy = entity.enemy_state;
  bool is_strong_attack = time_since(state.last_strong_attack_time) < 1.f;

  // Wand recoil against low guards
  // @todo magic piercing weapons
  state.last_wand_swing_time = 0.f;
  state.last_wand_bounce_time = get_scene_time();

  // Knockback
  if (is_strong_attack) {
    enemy.speed = -5000.f;
  } else {
    enemy.speed = -3000.f;
  }

  Sfx::PlaySound(SFX_WAND_RECOIL, 0.5f);
}

static void HandleLesserGuardWandStrike(Tachyon* tachyon, State& state, GameEntity& entity) {
  auto& enemy = entity.enemy_state;
  float scene_time = get_scene_time();
  bool is_blocking = time_since(enemy.last_block_time) < 1.f;
  bool is_invincible = time_since(enemy.last_damage_time) < 0.5f;
  bool is_strong_attack = time_since(state.last_strong_attack_time) < 1.f;
  bool is_active_target = state.has_target && IsSameEntity(entity, state.target_entity);

  if (is_blocking) {
    if (is_active_target || !state.has_target) {
      // Striking a blocking enemy
      state.last_wand_swing_time = 0.f;
      state.last_wand_bounce_time = scene_time;

      if (is_strong_attack) {
        // Breaking enemy defenses with a strong attack
        if (IsSameEntity(entity, state.target_entity)) {
          BreakEnemy(entity, scene_time);
          StunNearbyEnemies(state, scene_time);

          Sfx::PlaySound(SFX_SHIELD_BREAK, 0.5f);
        }
      } else {
        // Enemy knockback from wand bounce
        enemy.speed = -3000.f;

        Sfx::PlaySound(SFX_WAND_RECOIL, 0.5f);
      }
    }
  } else if (!is_invincible) {
    enemy.last_attack_start_time = 0.f;
    enemy.last_attack_action_time = 0.f;

    if (is_strong_attack) {
      // Breaking enemy defenses with a strong attack
      if (is_active_target) {
        BreakEnemy(entity, scene_time);
        StunNearbyEnemies(state, scene_time);

        Sfx::PlaySound(SFX_SHIELD_BREAK, 0.5f);
      }
    } else {
      // Normal attack damage + knockback
      float damage = time_since(enemy.last_break_time) < 1.f ? 50.f : 30.f;

      enemy.health -= damage;
      enemy.last_damage_time = scene_time;
      enemy.speed = -7000.f;

      Sfx::PlaySound(SFX_WAND_ATTACK, 0.5f);
    }

    if (enemy.health <= 0.f) {
      KillEnemy(entity, scene_time);

      if (IsSameEntity(entity, state.target_entity)) {
        state.has_target = false;

        Targeting::SelectNextAccessibleTarget(tachyon, state);
      }
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
      bool is_broken = time_since(enemy.last_break_time) < 1.5f;

      // @todo handle per enemy type (target.type)
      float attack_without_blocking_duration = 1.2f;

      if (enemy_distance < 4000.f && enemy.health > 0.f && !is_broken) {
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
    bool is_active_target = IsSameEntity(entity, state.target_entity);

    if (enemy_distance < 3000.f) {
      SetMood(entity, ENEMY_AGITATED, scene_time);

      if (entity.type == LOW_GUARD) {
        HandleLowGuardWandStrike(tachyon, state, entity);
      }
      else if (entity.type == LESSER_GUARD) {
        HandleLesserGuardWandStrike(tachyon, state, entity);
      }
    }
  }
}