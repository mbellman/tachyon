#include "astro/player_animation.h"
#include "astro/animation.h"
#include "astro/entity_behaviors/behavior.h"
#include "astro/entity_manager.h"
#include "astro/player_character.h"

using namespace astro;

static inline float GetAngleBetween(const float a1, const float a2) {
  float angle = a1 - a2;

  if (angle < -t_PI) angle += t_TAU;
  if (angle > t_PI) angle -= t_TAU;

  return angle;
}

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

static bool IsWalkAnimation(tSkeletonAnimation* animation, const State& state) {
  auto& animations = state.animations;

  return (
    animation == &animations.player_walk ||
    animation == &animations.player_walk_wand
  );
}

static bool IsRunAnimation(tSkeletonAnimation* animation, const State& state) {
  auto& animations = state.animations;

  return (
    animation == &animations.player_run ||
    animation == &animations.player_run_wand
  );
}

static bool ShouldPlayClimbingOffAnimation(Tachyon* tachyon, State& state) {
  float climbing_stop_time = state.player.last_climbing_stop_time;

  if (climbing_stop_time == 0.f) {
    return false;
  }

  if (state.did_climb_down) {
    return time_since(climbing_stop_time) < 0.4f;
  } else {
    return time_since(climbing_stop_time) < 1.3f;
  }
}

static bool ShouldPlayWalkAnimation(Tachyon* tachyon, State& state) {
  if (PlayerCharacter::IsClimbingOffLadder(tachyon, state)) {
    return false;
  }

  if (is_moving_left_stick()) {
    return true;
  }

  float walking_move_delta_threshold = state.use_slow_motion ? 2.f : 10.f;

  if (
    state.previous_move_delta > walking_move_delta_threshold &&
    state.player.rig.current_animation != &state.animations.player_climb_up
  ) {
    return true;
  }

  bool is_doing_quick_turn = (
    state.last_quick_turn_time != 0.f &&
    time_since(state.last_quick_turn_time) < 0.3f
  );

  bool has_target_and_is_moving = (
    state.has_target &&
    (tachyon->left_stick.x != 0.f || tachyon->left_stick.y != 0.f)
  );

  return is_doing_quick_turn || has_target_and_is_moving;
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

  float walking_move_delta_threshold = state.use_slow_motion ? 2.f : 10.f;

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
  else if (ShouldPlayClimbingOffAnimation(tachyon, state)) {
    if (state.did_climb_down) {
      Animation::StartNextAnimation(rig, &animations.player_climb_down);
    } else {
      Animation::StartNextAnimation(rig, &animations.player_climb_up);
    }
  }

  // Climbing
  else if (state.is_on_ladder) {
    if (state.is_starting_climb_down) {
      // @todo use a proper climb-down-onto-wall animation
      Animation::StartNextAnimation(rig, &animations.player_climb);
    } else {
      Animation::StartNextAnimation(rig, &animations.player_climb);
    }
  }

  // Quick-turning
  else if (time_since(state.last_quick_turn_time) < 0.15f) {
    Animation::StartNextAnimation(rig, &animations.player_idle_quickturn);
  }

  // Running
  else if (PlayerCharacter::IsRunning(tachyon, state)) {
    if (state.is_holding_up_wand) {
      Animation::AwaitNextAnimation(rig, &animations.player_run_wand);
    } else {
      Animation::AwaitNextAnimation(rig, &animations.player_run);
    }
  }

  // Walking
  else if (ShouldPlayWalkAnimation(tachyon, state)) {
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
      float seek_time = fmodf(rig.current_animation_time, 8.f);

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

static float GetAnimationSpeed(Tachyon* tachyon, State& state, tSkeletonAnimation* animation) {
  auto& rig = state.player.rig;
  auto& animations = state.animations;

  if (
    animation == &animations.player_idle ||
    animation == &animations.player_idle_2 ||
    animation == &animations.player_idle_wand ||
    animation == &animations.player_idle_wand_2
  ) {
    return 0.8f;
  }

  if (animation == &animations.player_idle_quickturn) {
    return 15.f;
  }

  float speed_ratio = state.player_velocity.magnitude() / PlayerCharacter::MAX_RUN_SPEED;

  if (
    animation == &animations.player_walk ||
    animation == &animations.player_walk_wand
  ) {
    return 13.5f * sqrt(speed_ratio);
  }

  if (
    animation == &animations.player_run ||
    animation == &animations.player_run_wand
  ) {
    return 12.f * sqrt(speed_ratio);
  }

  if (animation == &animations.player_climb_down) {
    return 7.f;
  }

  if (animation == &animations.player_climb_up) {
    return 8.5f;
  }

  if (animation == &animations.player_climb) {
    return is_moving_left_stick() ? 10.f : 0.f;
  }

  // @todo add remaining animations

  return 4.f;
}

static float GetAnimationBlendRate(Tachyon* tachyon, State& state) {
  auto& rig = state.player.rig;
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

  // Idle quickturn - > running
  if (
    rig.current_animation == &animations.player_idle_quickturn &&
    IsRunAnimation(rig.next_animation, state)
  ) {
    return 4.f;
  }

  // Blend faster if we're not currently in the running animation while running
  if (
    PlayerCharacter::IsRunning(tachyon, state) && (
      rig.current_animation != &animations.player_run &&
      rig.current_animation != &animations.player_run_wand
    )
  ) {
    return 4.f;
  }

  // Special case for blending into idle when climbing down
  // off a climbable wall. Use a slightly slower blend so we
  // transition into idle more naturally, mitigating awkward
  // foot shuffling behavior.
  // @todo check animation, rather than climbing stop time
  if (
    !state.is_on_ladder &&
    state.player.last_climbing_stop_time != 0.f &&
    time_since(state.player.last_climbing_stop_time) < 2.f &&
    state.did_climb_down
  ) {
    return 3.f;
  }

  // Blend faster into climbing
  if (state.is_on_ladder) {
    return 5.f;
  }

  // Blend faster into climbing up off a climbable surface
  if (PlayerCharacter::IsClimbingOffLadder(tachyon, state) && !state.did_climb_down) {
    return 5.f;
  }

  // Blend faster from running -> walking
  if (
    !PlayerCharacter::IsRunning(tachyon, state) &&
    IsWalkAnimation(rig.next_animation, state)
  ) {
    return 3.f;
  }

  // Blend faster out of idle
  if (
    IsAnyIdleAnimation(rig.current_animation, state) &&
    !IsAnyIdleAnimation(rig.next_animation, state)
  ) {
    return 3.5f;
  }

  // Blend faster into idle
  if (
    !IsAnyIdleAnimation(rig.current_animation, state) &&
    IsAnyIdleAnimation(rig.next_animation, state)
  ) {
    return 3.5f;
  }

  return 2.f;
}

static AnimationBlendType GetAnimationBlendType(State& state) {
  auto& rig = state.player.rig;

  if (HasCurrentWandAnimation(state) != HasNextWandAnimation(state)) {
    return BLEND_EASE_IN_OUT;
  }

  if (
    IsRunAnimation(rig.current_animation, state) &&
    IsWalkAnimation(rig.next_animation, state)
  ) {
    return BLEND_EASE_IN_OUT;
  }

  return BLEND_LINEAR;
}

static bool TurnHeadTowardPosition(State& state, const tVec3f& position, const float facing_angle) {
  tVec3f player_to_position = position - state.player_position;
  float position_direction_angle = atan2f(player_to_position.z, player_to_position.x);
  float turn = GetAngleBetween(position_direction_angle, facing_angle);

  if (abs(turn) > 1.8f) {
    return false;
  }

  if (turn < -1.6f) turn = -1.6f;
  if (turn > 1.6f) turn = 1.6f;

  auto& rig = state.player.rig;

  rig.head_turn_angle = Tachyon_Lerpf(rig.head_turn_angle, -turn, 5.f * state.dt);

  state.player.is_looking_at_something = true;

  return true;
}

static void TurnPlayerHeadToward(State& state, const std::vector<GameEntity>& entities, const float facing_angle) {
  for (auto& entity : entities) {
    if (!IsDuringActiveTime(entity, state)) continue;

    float entity_distance = tVec3f::distance(state.player_position, entity.visible_position);

    if (entity_distance > 7500.f) continue;

    if (TurnHeadTowardPosition(state, entity.visible_position, facing_angle)) {
      break;
    }
  }
}

static void HandleHeadAnimation(Tachyon* tachyon, State& state) {
  float player_facing_angle = atan2f(state.player_facing_direction.z, state.player_facing_direction.x);

  // Assume this is false until we determine otherwise
  state.player.is_looking_at_something = false;

  if (state.has_target) {
    // Turn head toward active target
    auto& entity = *EntityManager::FindEntity(state, state.target_entity);

    TurnHeadTowardPosition(state, entity.visible_position, player_facing_angle);
  }
  else if (state.preview_target_entity_record.type != UNSPECIFIED) {
    // Turn head toward preview target
    auto& entity = *EntityManager::FindEntity(state, state.preview_target_entity_record);

    TurnHeadTowardPosition(state, entity.visible_position, player_facing_angle);
  }
  else {
    // Turn head toward key entities when not targeting anything
    TurnPlayerHeadToward(state, state.sculpture_1s, player_facing_angle);
    TurnPlayerHeadToward(state, state.wind_chimes, player_facing_angle);
    TurnPlayerHeadToward(state, state.light_posts, player_facing_angle);

    // Turn head toward enemies + people
    if (!state.enemies_disabled) {
      TurnPlayerHeadToward(state, state.npcs, player_facing_angle);
      TurnPlayerHeadToward(state, state.low_guards, player_facing_angle);
      TurnPlayerHeadToward(state, state.lesser_guards, player_facing_angle);
    }

    // Turn head toward wand hints
    if (
      state.wand_hint_light_id != -1 &&
      time_since(state.last_wand_hint_time) < 4.f
    ) {
      auto& light = *get_point_light(state.wand_hint_light_id);
      tVec3f wand_hint_position = light.position;

      TurnHeadTowardPosition(state, wand_hint_position, player_facing_angle);
    }
  }

  // Turn head based on tilt for slightly more natural movement when changing direction
  {
    // Diminish the effect with velocity; otherwise when we
    // quick turn while running, the head takes unnaturally long
    // to rotate back into a normal forward orientation
    float alpha = 1.f - 0.75f * (state.player_velocity.magnitude() / PlayerCharacter::MAX_RUN_SPEED);

    state.player.rig.head_turn_angle += 10.f * alpha * state.tilt_angle * state.dt;
  }
}

static float GetRunningStopAlpha(const float running_stop) {
  if (running_stop < 0.33f) {
    return Tachyon_EaseInOutf(running_stop * 3.f);
  } else {
    return 1.f - Tachyon_EaseInOutf((running_stop - 0.33f) / 0.66f);
  }
}

static void HandleTorsoAnimation(Tachyon* tachyon, State& state) {
  auto& rig = state.player.rig;
  float player_speed = state.player_velocity.magnitude();
  float speed_ratio = player_speed / PlayerCharacter::MAX_RUN_SPEED;
  bool is_running = PlayerCharacter::IsRunning(tachyon, state);

  // Turn as we tilt
  if (state.tilt_angle != 0.f) {
    rig.torso_turn_angle = Tachyon_Lerpf(
      rig.torso_turn_angle,
      -state.tilt_angle,
      20.f * speed_ratio * state.dt
    );
  }

  // Turn a little when holding up the wand, inversely proportional
  // to speed (we want to reduce this effect when moving more quickly)
  {
    if (state.is_holding_up_wand) {
      float inverse_speed_ratio = 1.f - speed_ratio;

      rig.torso_turn_angle = Tachyon_Lerpf(
        rig.torso_turn_angle,
        0.5f,
        2.f * inverse_speed_ratio * state.dt
      );
    }
  }

  // Swing the torso while walking
  // @todo refactor/clean up
  float t = fmodf(rig.next_animation_time + 3.f, 8.f) / 8.f;
  float walk_cycle_time = t * t_TAU;
  float speed_curve_alpha = sinf(speed_ratio * t_PI);

  rig.torso_turn_angle += speed_curve_alpha * sinf(walk_cycle_time) * state.dt;

  // Swing when slowing down
  {
    if (speed_ratio > 0.f && !is_moving_left_stick()) {
      rig.torso_turn_angle = Tachyon_Lerpf(
        rig.torso_turn_angle,
        0.2f * sinf(walk_cycle_time),
        10.f * state.dt
      );
    }
  }

  // Lean forward when starting a run
  {
    if (is_running) {
      state.player.running_charge += state.dt;

      clamp_to_1(state.player.running_charge);
    } else {
      if (state.player.running_charge == 1.f) {
        state.player.running_charge = 0.f;
      }

      state.player.running_charge *= 1.f - 2.f * state.dt;
    }

    float running_charge_tilt = 0.1f * sinf(state.player.running_charge * t_PI);

    if (running_charge_tilt > 0.f) {
      rig.torso_tilt_angle = Tachyon_Lerpf(
        rig.torso_tilt_angle,
        running_charge_tilt,
        10.f * state.dt
      );
    }
  }

  // Lean backward when moving quickly down slopes
  {
    if (speed_ratio > 0.75f && state.is_moving_down_slope) {
      rig.torso_tilt_angle = Tachyon_Lerpf(
        rig.torso_tilt_angle,
        -0.25f,
        3.f * state.dt
      );
    }
  }

  // Compress when running
  {
    float t = fmodf(rig.next_animation_time + 3.5f, 8.f) / 8.f;
    float alpha = 0.5f + 0.5f * sinf(2.f * t * t_TAU);
    float compression = 0.5f * alpha;

    if (PlayerCharacter::IsRunning(tachyon, state)) {
      rig.torso_compression = speed_ratio * compression;
    } else {
      rig.torso_compression = Tachyon_Lerpf(rig.torso_compression, 0.f, 0.2f * state.dt);
    }
  }
}

static void HandleLookAroundAnimation(Tachyon* tachyon, State& state, float t, float blend) {
  auto& rig = state.player.rig;
  float oscillation = sinf(t);

  rig.head_turn_angle = Tachyon_Lerpf(rig.head_turn_angle, 0.5f * oscillation, blend);
  rig.torso_turn_angle = Tachyon_Lerpf(rig.torso_turn_angle, 0.25f * oscillation, blend);
}

static void HandleStoppedAfterMovingAnimation(Tachyon* tachyon, State& state) {
  // Do nothing if:
  if (
    // We're looking at something
    state.player.is_looking_at_something ||
    // We haven't manually stopped yet
    state.player.last_stopped_moving_time == 0.f ||
    // We're on a ladder, or climbing off
    state.is_on_ladder || PlayerCharacter::IsClimbingOffLadder(tachyon, state)
  ) {
    return;
  }

  float did_just_climb_down = (
    time_since(state.player.last_climbing_time) < 4.f &&
    state.did_climb_down
  );

  // Don't perform stopped moving animation after climbing down off something
  if (did_just_climb_down) {
    return;
  }

  float time_since_stopped_moving = time_since(state.player.last_stopped_moving_time);

  // When stopping, trigger look-around behavior for a few seconds
  if (time_since_stopped_moving < 2.75f) {
    float animation_time = 2.f * time_since_stopped_moving;
    float blend = time_since_stopped_moving / 1.5f;

    clamp_to_1(blend);

    HandleLookAroundAnimation(tachyon, state, animation_time, blend);
  }
}

static void DriftToRestAnimation(Tachyon* tachyon, State& state) {
  auto& rig = state.player.rig;

  rig.head_turn_angle = Tachyon_Lerpf(rig.head_turn_angle, 0.f, 4.f * state.dt);
  rig.torso_turn_angle = Tachyon_Lerpf(rig.torso_turn_angle, 0.f, 0.5f * state.dt);
  rig.torso_tilt_angle = Tachyon_Lerpf(rig.torso_tilt_angle, 0.f, 0.5f * state.dt);
}

void PlayerAnimation::Update(Tachyon* tachyon, State& state) {
  profile("PlayerAnimation::Update()");

  auto& rig = state.player.rig;
  auto& animations = state.animations;

  SetActiveAnimation(tachyon, state);

  if (
    (IsWalkAnimation(rig.current_animation, state) || IsRunAnimation(rig.current_animation, state)) &&
    (IsWalkAnimation(rig.next_animation, state) || IsRunAnimation(rig.next_animation, state))
  ) {
    rig.next_animation_time = rig.current_animation_time;
  }

  bool moving_forward = tVec3f::dot(state.player_velocity, state.player_facing_direction) >= 0.f;
  float blend_rate = GetAnimationBlendRate(tachyon, state);
  auto blend_type = GetAnimationBlendType(state);

  rig.current_animation_speed = GetAnimationSpeed(tachyon, state, rig.current_animation);
  rig.next_animation_speed = GetAnimationSpeed(tachyon, state, rig.next_animation);

  // When manually moving backward, or climbing down a ladder, play the animation in reverse
  if (
    time_since(state.last_quick_turn_time) > 0.5f && (
      (!moving_forward && !PlayerCharacter::IsClimbingOffLadder(tachyon, state)) ||
      (state.is_on_ladder && tachyon->left_stick.y > 0.f) ||
      state.is_starting_climb_down
    )
  ) {
    rig.current_animation_speed *= -1.f;
    rig.next_animation_speed *= -1.f;
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

      rig.upper_body_animation = &swing_animation;
      rig.upper_body_animation_time = speed * alpha;
    } else {
      rig.upper_body_animation = nullptr;
    }
  }

  HandleHeadAnimation(tachyon, state);
  HandleTorsoAnimation(tachyon, state);
  HandleStoppedAfterMovingAnimation(tachyon, state);
  DriftToRestAnimation(tachyon, state);

  Animation::AccumulateTime(state.player.rig, blend_rate, state.dt);
  Animation::UpdatePose(state.player.rig, blend_type);
  Animation::UpdateBoneMatrices(state.player.rig);
}

float PlayerAnimation::GetRunCycleAnimationTime(State& state) {
  auto& animations = state.animations;
  auto& rig = state.player.rig;

  // If our next animation is walking or running, use that
  // as our source value. This can also mean both current and
  // next use the same animation, but we want to favor the
  // next animation if they are not.
  if (
    rig.next_animation == &animations.player_walk ||
    rig.next_animation == &animations.player_walk_wand ||
    rig.next_animation == &animations.player_run ||
    rig.next_animation == &animations.player_run_wand
  ) {
    return rig.next_animation_time;
  }

  // If our next animation is something other than walking or running,
  // use the residual animation time from our current animation as we
  // blend into that one.
  if (
    rig.current_animation == &animations.player_walk ||
    rig.current_animation == &animations.player_walk_wand ||
    rig.current_animation == &animations.player_run ||
    rig.current_animation == &animations.player_run_wand
  ) {
    return rig.current_animation_time;
  }

  return 0.f;
}