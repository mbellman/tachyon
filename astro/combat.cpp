#include "astro/combat.h"
#include "astro/entity_behaviors/behavior.h"
#include "astro/entity_manager.h"
#include "astro/sfx.h"

using namespace astro;

static bool TestForStrongAttack(Tachyon* tachyon, State& state) {
  if (state.has_target) {
    auto& target = *EntityManager::FindEntity(state, state.target_entity);
    float time_since_dodging = time_since(state.last_dodge_time);
    float time_since_enemy_attack_action = time_since(target.enemy_state.last_attack_action_time);
    float time_since_taking_damage = time_since(state.last_damage_time);

    if (
      time_since_dodging < 1.f &&
      time_since_enemy_attack_action < 0.3f &&
      time_since_taking_damage > 1.f
    ) {
      float target_distance = tVec3f::distance(target.visible_position, state.player_position);
      tVec3f direction_to_target = (target.visible_position - state.player_position) / target_distance;

      state.player_velocity = direction_to_target * target_distance;
      state.last_strong_attack_time = get_scene_time();
      state.last_dodge_time = 0.f;

      auto& enemy = target.enemy_state;

      // @temporary
      if (target.type == LESSER_GUARD) {
        enemy.health -= 50.f;
      }
      else if (target.type == LOW_GUARD) {
        // Armor block
        // @todo allow magical piercing attacks
        enemy.last_block_time = get_scene_time();
      }

      if (enemy.health <= 0.f) {
        KillEnemy(target, get_scene_time());
      }

      return true;
    }
  }

  return false;
}

void Combat::HandleWandSwing(Tachyon* tachyon, State& state) {
  // Check against nearby targets
  {
    for (auto& target : state.targetable_entities) {
      auto& entity = *EntityManager::FindEntity(state, target);
      auto& enemy = entity.enemy_state;
      float distance_from_player = tVec3f::distance(entity.visible_position, state.player_position);

      // @todo handle per enemy type (target.type)
      float attack_without_blocking_duration = 1.2f;

      if (distance_from_player < 4000.f && enemy.health > 0.f) {
        if (time_since(enemy.last_attack_start_time) > attack_without_blocking_duration) {
          // Block
          enemy.last_block_time = get_scene_time();
        } else if (entity.type == LOW_GUARD) {
          // Armor block
          // @todo allow magical piercing attacks
          enemy.last_block_time = get_scene_time();
        } else if (distance_from_player < 3500.f) {
          // @temporary
          if (entity.type == LESSER_GUARD) {
            enemy.health -= 30.f;
          }

          // @temporary
          Sfx::PlaySound(SFX_WAND_ATTACK, 0.5f);

          if (enemy.health <= 0.f) {
            KillEnemy(entity, get_scene_time());
          }
        }
      }
    }
  }

  // Distinguishing between strong and regular attacks
  {
    if (TestForStrongAttack(tachyon, state)) {
      Sfx::PlaySound(SFX_WAND_STRONG_ATTACK, 0.3f);
    } else {
      Sfx::PlaySound(SFX_WAND_SWING, 0.3f);
    }
  }
}

// @todo handle recoil against solid objects
void Combat::HandleWandStrikeWindow(Tachyon* tachyon, State& state) {
  for (auto& target : state.targetable_entities) {
    auto& entity = *EntityManager::FindEntity(state, target);
    float enemy_distance = tVec3f::distance(entity.visible_position, state.player_position);
    float time_since_enemy_blocked = time_since(entity.enemy_state.last_block_time);

    if (enemy_distance < 3000.f) {
      if (time_since_enemy_blocked < 1.f) {
        // Wand recoil when the enemy is blocking
        state.last_wand_swing_time = 0.f;
        state.last_wand_bounce_time = get_scene_time();

        Sfx::PlaySound(SFX_WAND_RECOIL, 0.5f);
      } else {
        // Enemy knockback on contact
        entity.enemy_state.speed = -7000.f;

        // @todo take damage here instead of in above functions?
      }
    }
  }
}