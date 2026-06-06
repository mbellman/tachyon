#include "astro/player_animation.h"
#include "astro/animation.h"
#include "astro/player_character.h"

using namespace astro;

static bool HasCurrentWandAnimation(State& state) {
  auto& animations = state.animations;
  auto& rig = state.player.rig;

  return (
    rig.current_animation == &animations.player_idle_wand ||
    rig.current_animation == &animations.player_walk_wand ||
    rig.current_animation == &animations.player_run_wand
  );
}

static bool HasNextWandAnimation(State& state) {
  auto& animations = state.animations;
  auto& rig = state.player.rig;

  return (
    rig.next_animation == &animations.player_idle_wand ||
    rig.next_animation == &animations.player_walk_wand ||
    rig.next_animation == &animations.player_run_wand
  );
}

static bool IsNormalIdleAnimation(tSkeletonAnimation* animation, const State& state) {
  auto& animations = state.animations;

  return (
    animation == &animations.player_idle ||
    animation == &animations.player_idle_2
  );
}

static bool IsWandIdleAnimation(tSkeletonAnimation* animation, const State& state) {
  auto& animations = state.animations;

  return (
    animation == &animations.player_idle_wand
  );
}

static bool IsAnyIdleAnimation(tSkeletonAnimation* animation, const State& state) {
  return IsNormalIdleAnimation(animation, state) || IsWandIdleAnimation(animation, state);
}

static void SetActiveAnimation(Tachyon* tachyon, State& state) {
  auto& rig = state.player.rig;
  auto& animations = state.animations;

  bool is_doing_quick_turn = (
    state.last_quick_turn_time != 0.f &&
    time_since(state.last_quick_turn_time) < 0.3f
  );

  bool has_target_and_is_moving = (
    state.has_target &&
    (tachyon->left_stick.x != 0.f || tachyon->left_stick.y != 0.f)
  );

  // Set the default current animation if not initialized
  if (rig.current_animation == nullptr) {
    rig.current_animation = &animations.player_idle;
  }

  // Taking damage
  if (
    state.last_damage_time != 0.f &&
    time_since(state.last_damage_time) < 1.f
  ) {
    Animation::StartNextAnimation(rig, &animations.person_hit_front);
  }

  // Astro traveling
  else if (state.astro_turn_speed != 0.f) {
    Animation::AwaitNextAnimation(rig, &animations.player_idle_wand);
  }

  // Climbing off
  else if (
    state.last_off_ladder_time != 0.f &&
    time_since(state.last_off_ladder_time) < 0.5f
  ) {
    if (state.did_climb_down) {
      Animation::StartNextAnimation(rig, &animations.player_climb_down);
    } else {
      Animation::StartNextAnimation(rig, &animations.player_climb_up);
    }
  }

  // Climbing
  else if (state.is_on_ladder) {
    if (state.is_starting_climb_down) {
      Animation::StartNextAnimation(rig, &animations.player_climb_up);
    } else {
      Animation::AwaitNextAnimation(rig, &animations.player_climb);
    }
  }

  // Running
  else if (PlayerCharacter::IsRunning(tachyon, state)) {
    if (state.is_holding_up_wand) {
      Animation::AwaitNextAnimation(rig, &animations.player_run_wand);
    } else {
      if (
        IsWandIdleAnimation(rig.current_animation, state) &&
        IsNormalIdleAnimation(rig.next_animation, state)
      ) {
        // Special case for lowering the wand while idle, and immediately
        // starting a run action. Ordinarily, this would cause us to wait
        // for the transition from wand idle -> idle before then starting
        // the run animation, causing awkward gliding behavior while still
        // using the idle animation. Instead, we "cancel" and immediately
        // start transitioning to the run animation.
        //
        // @todo factor this into an animation cancel method (?)
        float current_blend = rig.next_animation_blend_alpha;

        Animation::SetNextAnimation(rig, &animations.player_run);

        rig.next_animation_blend_alpha = current_blend;
      } else {
        Animation::AwaitNextAnimation(rig, &animations.player_run);
      }
    }
  }

  // Walking
  else if (
    (
      time_since(state.last_off_ladder_time) > 1.5f ||
      is_moving_left_stick()
    ) && (
      state.previous_move_delta > 10.f ||
      is_doing_quick_turn ||
      has_target_and_is_moving
    )
  ) {
    if (state.is_holding_up_wand) {
      Animation::AwaitNextAnimation(rig, &animations.player_walk_wand);
    } else {
      Animation::AwaitNextAnimation(rig, &animations.player_walk);
    }
  }

  // Idling
  else {

    // Just before we switch to the idle animation, determine where we are
    // in our walk cycle, and decide on an idle stance based on that. The two
    // idle stances use opposite foot positioning, which are alternately picked
    // to reduce foot sliding from walk -> idle.
    if (
      rig.next_animation == &animations.player_walk ||
      rig.next_animation == &animations.player_walk_wand
    ) {
      float seek_time = fmodf(rig.seek_time, 8.f);

      if (seek_time < 1.5f || seek_time > 6.5f) {
        state.player_idle_stance = 2;
      } else {
        state.player_idle_stance = 1;
      }
    }

    // If we just climbed off a ladder, default to stance 2,
    // which better matches the animation
    if (PlayerCharacter::IsClimbingOffLadder(tachyon, state)) {
      state.player_idle_stance = 2;
    }

    if (state.is_holding_up_wand) {
      Animation::AwaitNextAnimation(rig, &animations.player_idle_wand);
    } else if (state.player_idle_stance == 1) {
      Animation::AwaitNextAnimation(rig, &animations.player_idle);
    } else if (state.player_idle_stance == 2) {
      Animation::AwaitNextAnimation(rig, &animations.player_idle_2);
    }
  }
}


static float GetAnimationSpeed(Tachyon* tachyon, State& state) {
  bool is_astro_traveling = state.astro_turn_speed != 0.f;
  bool is_hit = state.last_damage_time != 0.f && time_since(state.last_damage_time) < 1.f;
  bool is_idle = IsAnyIdleAnimation(state.player.rig.next_animation, state);

  if (is_astro_traveling) return 0.65f;
  if (is_hit) return 7.f;
  if (is_idle) return 0.8f;

  if (state.is_on_ladder) {
    return is_moving_left_stick() || state.is_starting_climb_down ? 8.f : 0.f;
  }

  if (PlayerCharacter::IsClimbingOffLadder(tachyon, state)) {
    return state.did_climb_down ? 7.f : 8.f;
  }

  float player_speed = state.player_velocity.magnitude();
  float max_walk_speed = state.has_target ? PlayerCharacter::MAX_COMBAT_WALK_SPEED : PlayerCharacter::MAX_WALK_SPEED;
  float speed_ratio = player_speed / PlayerCharacter::MAX_RUN_SPEED;
  bool is_running = player_speed > max_walk_speed;

  return (is_running ? 11.5f : 13.5f) * sqrtf(speed_ratio);
}

static float GetAnimationBlendRate(Tachyon* tachyon, State& state) {
  auto& player_animation = state.player.rig;
  auto& animations = state.animations;

  // If our current and pending animation involves holding up the wand,
  // but we're not longer holding it, speed up the current blend
  // so we transition out of the idle/walk/run-with-wand animation
  // more quickly.
  if (
    !state.is_holding_up_wand &&
    HasCurrentWandAnimation(state) &&
    HasNextWandAnimation(state)
  ) {
    return 10.f;
  }

  // If our current and pending animations don't involve holding the wand,
  // but we're trying to hold it up, speed up the current blend so we can
  // get there faster.
  if (
    state.is_holding_up_wand &&
    !HasCurrentWandAnimation(state) &&
    !HasNextWandAnimation(state)
  ) {
    return 5.f;
  }

  // Blend faster if we're not currently in the running animation while running
  if (
    PlayerCharacter::IsRunning(tachyon, state) && (
      player_animation.current_animation != &animations.player_run &&
      player_animation.current_animation != &animations.player_run_wand
    )
  ) {
    return 5.f;
  }

  // Special case for blending into idle when climbing down
  // off a climbable wall. Use a slightly slower blend so we
  // transition into idle more naturally, mitigating awkward
  // foot shuffling behavior.
  if (
    !state.is_on_ladder &&
    state.last_off_ladder_time != 0.f &&
    time_since(state.last_off_ladder_time) < 1.5f &&
    state.did_climb_down
  ) {
    return 2.f;
  }

  // Blend faster out of idle
  if (
    IsAnyIdleAnimation(player_animation.current_animation, state) &&
    !IsAnyIdleAnimation(player_animation.next_animation, state)
  ) {
    return 3.5f;
  }

  // Blend faster into idle
  if (
    !IsAnyIdleAnimation(player_animation.current_animation, state) &&
    IsAnyIdleAnimation(player_animation.next_animation, state)
  ) {
    return 3.5f;
  }

  return 2.f;
}

static AnimationBlendType GetAnimationBlendType(State& state) {
  if (HasCurrentWandAnimation(state) != HasNextWandAnimation(state)) {
    return BLEND_EASE_IN_OUT;
  }

  return BLEND_LINEAR;
}

void PlayerAnimation::Update(Tachyon* tachyon, State& state) {
  profile("PlayerAnimation::Update()");

  auto& player_animation = state.player.rig;
  auto& animations = state.animations;

  SetActiveAnimation(tachyon, state);

  bool moving_forward = tVec3f::dot(state.player_velocity, state.player_facing_direction) >= 0.f;
  float animation_speed = GetAnimationSpeed(tachyon, state);
  float blend_rate = GetAnimationBlendRate(tachyon, state);
  auto blend_type = GetAnimationBlendType(state);

  // When manually moving backward, or climbing down a ladder, play the animation in reverse
  if (
    (!moving_forward && !PlayerCharacter::IsClimbingOffLadder(tachyon, state)) ||
    (state.is_on_ladder && tachyon->left_stick.y > 0.f) ||
    state.is_starting_climb_down
  ) {
    animation_speed *= -1.f;
  }

  // @todo factor
  {
    auto& swing_animation = animations.player_swing_wand;
    float frame_duration = 0.15f;
    float animation_duration = frame_duration * float(swing_animation.frames.size());

    if (
      state.last_wand_swing_time != 0.f &&
      time_since(state.last_wand_swing_time) < animation_duration
    ) {
      float speed = animation_duration / frame_duration;
      float alpha = time_since(state.last_wand_swing_time) / animation_duration;

      player_animation.upper_body_animation = &swing_animation;
      player_animation.upper_body_animation_time = speed * alpha;
    } else {
      player_animation.upper_body_animation = nullptr;
    }
  }

  Animation::AccumulateTime(state.player.rig, animation_speed, blend_rate, state.dt);
  Animation::UpdatePose(state.player.rig, blend_type);
  Animation::UpdateBoneMatrices(state.player.rig);
}