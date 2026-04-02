#include "astro/combat.h"
#include "astro/entity_behaviors/behavior.h"
#include "astro/entity_manager.h"
#include "astro/player_character.h"
#include "astro/sfx.h"
#include "astro/targeting.h"

using namespace astro;

static void BreakEnemy(GameEntity& entity, const float scene_time) {
  auto& enemy = entity.enemy_state;

  enemy.speed = -4000.f;
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

static void PlayMetalHitSound() {
  // @todo refactor
  float r = Tachyon_GetRandom();

  if (r < 0.25f) Sfx::PlaySound(SFX_METAL_HIT_1, 0.5f);
  else if (r < 0.5f) Sfx::PlaySound(SFX_METAL_HIT_2, 0.5f);
  else if (r < 0.75f) Sfx::PlaySound(SFX_METAL_HIT_3, 0.5f);
  else Sfx::PlaySound(SFX_METAL_HIT_4, 0.5f);
}

static void HandleLowGuardWandStrike(Tachyon* tachyon, State& state, GameEntity& entity) {
  auto& enemy = entity.enemy_state;
  bool is_player_doing_break_attack = time_since(state.last_break_attack_time) < 0.5f;

  // Wand recoil against low guards
  // @todo magic piercing weapons
  state.last_wand_swing_time = 0.f;
  state.last_wand_bounce_time = get_scene_time();

  // Knockback
  if (is_player_doing_break_attack) {
    enemy.speed = -5000.f;
  } else {
    enemy.speed = -3000.f;
  }

  PlayerCharacter::GetKnockedBack(state, 2000.f);

  PlayMetalHitSound();
}

static void HandleLesserGuardWandStrike(Tachyon* tachyon, State& state, GameEntity& entity) {
  auto& enemy = entity.enemy_state;
  float scene_time = get_scene_time();
  float time_since_starting_attack = time_since(enemy.last_attack_start_time);
  bool is_enemy_blocking = time_since(enemy.last_block_time) < 1.f;
  bool is_enemy_on_damage_cooldown = time_since(enemy.last_damage_time) < 0.5f;
  bool is_enemy_preparing_attack = time_since_starting_attack < 1.5f;
  bool is_player_doing_break_attack = time_since(state.last_break_attack_time) < 0.5f;
  bool is_active_target = state.has_target && IsSameEntity(entity, state.target_entity);

  // Striking an attacking enemy
  if (is_enemy_preparing_attack) {
    bool is_attack_parryable = time_since_starting_attack > 0.7f;
    float facing_dot = tVec3f::dot(state.player_facing_direction, GetFacingDirection(entity));

    // Parrying
    if (is_attack_parryable && facing_dot < 0.f) {
      // @todo ParryEnemy()
      BreakEnemy(entity, scene_time);

      PlayMetalHitSound();
    }
  }

  // Striking a blocking enemy
  else if (is_enemy_blocking) {
    // Only count hits against active targets
    // or in non-targeting scenarios
    if (is_active_target || !state.has_target) {
      state.last_wand_swing_time = 0.f;
      state.last_wand_bounce_time = scene_time;

      if (is_player_doing_break_attack) {
        // Breaking enemy defenses
        if (IsSameEntity(entity, state.target_entity)) {
          BreakEnemy(entity, scene_time);
          StunNearbyEnemies(state, scene_time);

          PlayerCharacter::GetKnockedBack(state, 750.f);

          Sfx::PlaySound(SFX_SHIELD_BREAK, 0.5f);
        }
      } else {
        // Enemy knockback from wand bounce
        enemy.speed = -3000.f;

        PlayerCharacter::GetKnockedBack(state, 2000.f);

        PlayMetalHitSound();
      }
    }

  // Striking a non-blocking enemy
  } else if (!is_enemy_on_damage_cooldown) {
    enemy.last_attack_start_time = 0.f;
    enemy.last_attack_action_time = 0.f;

    if (is_player_doing_break_attack) {
      if (is_active_target) {
        // Break attack against a non-blocking enemy
        state.last_wand_swing_time = 0.f;
        state.last_wand_bounce_time = scene_time;

        BreakEnemy(entity, scene_time);
        StunNearbyEnemies(state, scene_time);

        state.player_velocity = tVec3f(0.f);

        Sfx::PlaySound(SFX_SHIELD_BREAK, 0.5f);
      }
    } else {
      // Attack damage + knockback
      bool is_enemy_broken = time_since(enemy.last_break_time) < 2.f;
      float damage = is_enemy_broken ? 50.f : 30.f;
      float knockback = is_enemy_broken ? -10000.f : -7000.f;

      enemy.health -= damage;
      enemy.last_damage_time = scene_time;
      enemy.speed = knockback;

      if (is_enemy_broken) {
        Sfx::PlaySound(SFX_STRONG_ATTACK, 0.5f);
      } else {
        Sfx::PlaySound(SFX_WAND_ATTACK, 0.5f);
      }
    }

    if (enemy.health <= 0.f) {
      KillEnemy(entity, scene_time);

      Sfx::PlaySound(SFX_SWORD_DROP, 0.5f);

      if (IsSameEntity(entity, state.target_entity)) {
        // Allow the targeting system to fall back to its non-targeted
        // selection scheme, ensuring that we skip over the just-killed enemy
        state.has_target = false;

        Targeting::SelectNextAccessibleTarget(tachyon, state);
      }
    }
  }
}

void Combat::HandleWandSwing(Tachyon* tachyon, State& state) {
  float scene_time = get_scene_time();
  bool did_player_recently_break_attack = time_since(state.last_break_attack_time) < 2.f;

  // Triggering nearby targets into blocking
  {
    for (auto& target : state.targetable_entities) {
      auto& entity = *EntityManager::FindEntity(state, target);
      auto& enemy = entity.enemy_state;
      float enemy_distance = tVec3f::distance(entity.visible_position, state.player_position);
      bool is_enemy_broken = time_since(enemy.last_break_time) < 2.f;

      // @todo handle per enemy type (target.type)
      float attack_without_blocking_duration = 0.6f;
      bool can_enemy_cancel_attack_to_block = time_since(enemy.last_attack_start_time) < attack_without_blocking_duration;

      if (
        enemy_distance < 5000.f &&
        enemy.health > 0.f &&
        !is_enemy_broken &&
        // Allow the player to do post-break attacks without other enemies blocking
        !did_player_recently_break_attack &&
        can_enemy_cancel_attack_to_block
      ) {
        // @todo factor by enemy type
        if (entity.type == LOW_GUARD) {
          // Armor blocking
          // @todo magic weapons which can pierce armor
          enemy.last_block_time = scene_time;
        }
        else if (entity.type == LESSER_GUARD) {
          float facing_dot = tVec3f::dot(state.player_facing_direction, GetFacingDirection(entity));

          // Block when facing the player
          if (facing_dot < 0.f) {
            enemy.last_block_time = scene_time;
            enemy.last_attack_start_time = 0.f;
            enemy.last_attack_action_time = 0.f;
          }
        }
      }
    }
  }

  // Distinguishing between break and regular attacks
  {
    if (time_since(state.last_target_jump_time) < 0.5f) {
      state.last_break_attack_time = scene_time;

      Sfx::PlaySound(SFX_BREAK_ATTACK, 0.3f);
    } else {
      Sfx::PlaySound(SFX_WAND_SWING, 0.3f);
    }
  }
}

// @todo handle recoil against solid objects
void Combat::HandleWandStrikeWindow(Tachyon* tachyon, State& state) {
  float scene_time = get_scene_time();

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