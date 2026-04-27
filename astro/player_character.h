#pragma once

#include "astro/game_state.h"

namespace astro {
  namespace PlayerCharacter {
    const static float MAX_RUN_SPEED = 1500.f;
    const static float MAX_COMBAT_WALK_SPEED = 800.f;
    const static float MAX_WALK_SPEED = 550.f;

    void UpdatePlayer(Tachyon* tachyon, State& state);
    void AutoHop(Tachyon* tachyon, State& state);
    bool CanTakeDamage(Tachyon* tachyon, const State& state);
    bool IsRunning(Tachyon* tachyon, State& state);
    float GetHumanEnemyAlertedSpeed(const State& state);
    void TakeDamage(Tachyon* tachyon, State& state, const float damage);
    void GetKnockedBack(State& state, float speed);
    void PerformStandardDodgeAction(Tachyon* tachyon, State& state);
    void PerformTargetJumpAction(Tachyon* tachyon, State& state);
  }
}