#include "astro/player_character.h"
#include "astro/animation.h"
#include "astro/astrolabe.h"
#include "astro/combat.h"
#include "astro/entity_behaviors/behavior.h"
#include "astro/entity_manager.h"
#include "astro/items.h"
#include "astro/magic.h"
#include "astro/sfx.h"
#include "astro/simple_animation.h"
#include "astro/time_evolution.h"
#include "astro/ui_system.h"

using namespace astro;

constexpr static float ATTACK_WIND_UP_DURATION = 0.3f;
constexpr static float ATTACK_SWING_DURATION = 0.2f;
constexpr static float ATTACK_WIND_DOWN_DURATION = 0.6f;
constexpr static float ATTACK_DURATION = ATTACK_WIND_UP_DURATION + ATTACK_SWING_DURATION + ATTACK_WIND_DOWN_DURATION;

constexpr static float AUTO_HOP_DURATION = 0.3f;

static inline float GetAngleBetween(const float a1, const float a2) {
  float angle = a1 - a2;

  if (angle < -t_PI) angle += t_TAU;
  if (angle > t_PI) angle -= t_TAU;

  return angle;
}

static bool HasCurrentWandAnimation(State& state) {
  auto& animations = state.animations;
  auto& player_animation = state.player_mesh_animation;

  return (
    player_animation.current_animation == &animations.player_idle_wand ||
    player_animation.current_animation == &animations.player_walk_wand ||
    player_animation.current_animation == &animations.player_run_wand
  );
}

static bool HasNextWandAnimation(State& state) {
  auto& animations = state.animations;
  auto& player_animation = state.player_mesh_animation;

  return (
    player_animation.next_animation == &animations.player_idle_wand ||
    player_animation.next_animation == &animations.player_walk_wand ||
    player_animation.next_animation == &animations.player_run_wand
  );
}

static void HandleActiveAnimation(Tachyon* tachyon, State& state) {
  auto& player_animation = state.player_mesh_animation;
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
  if (player_animation.current_animation == nullptr) {
    player_animation.current_animation = &animations.player_idle;
  }

  // Taking damage
  if (
    state.last_damage_time != 0.f &&
    time_since(state.last_damage_time) < 1.f
  ) {
    Animation::StartNextAnimation(player_animation, &animations.person_hit_front);
  }

  // Astro traveling
  else if (state.astro_turn_speed != 0.f) {
    Animation::AwaitNextAnimation(player_animation, &animations.player_idle_wand);
  }

  // Running
  else if (PlayerCharacter::IsRunning(tachyon, state)) {
    if (state.wand_hold_factor == 1.f) {
      Animation::AwaitNextAnimation(player_animation, &animations.player_run_wand);
    } else {
      if (
        player_animation.current_animation == &animations.player_idle_wand &&
        player_animation.next_animation == &animations.player_idle
      ) {
        // Special case for lowering the wand while idle, and immediately
        // starting a run action. Ordinarily, this would cause us to wait
        // for the transition from wand idle -> idle before then starting
        // the run animation, causing awkward gliding behavior while still
        // using the idle animation. Instead, we "cancel" and immediately
        // start transitioning to the run animation.
        //
        // @todo factor this into an animation cancel method (?)
        float current_blend = player_animation.next_animation_blend_alpha;

        Animation::SetNextAnimation(player_animation, &animations.player_run);

        player_animation.next_animation_blend_alpha = current_blend;
      } else {
        Animation::AwaitNextAnimation(player_animation, &animations.player_run);
      }
    }
  }

  // Walking
  else if (state.previous_move_delta > 5.f || is_doing_quick_turn || has_target_and_is_moving) {
    if (state.wand_hold_factor == 1.f) {
      Animation::AwaitNextAnimation(player_animation, &animations.player_walk_wand);
    } else {
      Animation::AwaitNextAnimation(player_animation, &animations.player_walk);
    }
  }

  // Idling
  else {
    if (state.wand_hold_factor == 1.f) {
      Animation::AwaitNextAnimation(player_animation, &animations.player_idle_wand);
    } else {
      Animation::AwaitNextAnimation(player_animation, &animations.player_idle);
    }
  }
}

static float GetAnimationSpeed(Tachyon* tachyon, State& state) {
  bool is_astro_traveling = state.astro_turn_speed != 0.f;
  bool is_hit = state.last_damage_time != 0.f && time_since(state.last_damage_time) < 1.f;

  bool is_idle = (
    state.player_mesh_animation.next_animation == &state.animations.player_idle ||
    state.player_mesh_animation.next_animation == &state.animations.player_idle_wand
  );

  if (is_astro_traveling) return 0.65f;
  if (is_hit) return 7.f;
  if (is_idle) return 0.8f;

  float player_speed = state.player_velocity.magnitude();
  float max_walk_speed = state.has_target ? PlayerCharacter::MAX_COMBAT_WALK_SPEED : PlayerCharacter::MAX_WALK_SPEED;
  float speed_ratio = player_speed / PlayerCharacter::MAX_RUN_SPEED;
  bool is_running = player_speed > max_walk_speed;

  return (is_running ? 11.5f : 12.f) * sqrtf(speed_ratio);
}

static float GetAnimationBlendRate(Tachyon* tachyon, State& state) {
  auto& player_animation = state.player_mesh_animation;
  auto& animations = state.animations;

  // If our current or pending animation involves holding the wand,
  // but we're not longer holding it, speed up the current blend
  // so we transition out of the idle/walk/run-with-wand animation
  // more quickly.
  if (
    state.wand_hold_factor < 1.f &&
    HasCurrentWandAnimation(state) &&
    HasNextWandAnimation(state)
  ) {
    return 10.f;
  }

  // Blend more slowly into wand holding actions
  if (
    !HasCurrentWandAnimation(state) &&
    HasNextWandAnimation(state)
  ) {
    return 1.5f;
  }

  // Blend more slowly out of wand holding actions
  if (
    HasCurrentWandAnimation(state) &&
    !HasNextWandAnimation(state)
  ) {
    return 1.5f;
  }

  if (
    PlayerCharacter::IsRunning(tachyon, state) &&
    player_animation.current_animation != &animations.player_run
  ) {
    return 8.f;
  }

  if (
    player_animation.current_animation == &animations.player_idle &&
    player_animation.next_animation == &animations.player_run
  ) {
    return 3.5f;
  }

  return 2.f;
}

static void UpdatePlayerSkeleton(Tachyon* tachyon, State& state) {
  profile("UpdatePlayerSkeleton()");

  auto& player_animation = state.player_mesh_animation;
  auto& animations = state.animations;

  HandleActiveAnimation(tachyon, state);

  bool moving_forward = tVec3f::dot(state.player_velocity, state.player_facing_direction) >= 0.f;
  float animation_speed = GetAnimationSpeed(tachyon, state);
  float blend_rate = GetAnimationBlendRate(tachyon, state);

  if (!moving_forward) {
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

  Animation::AccumulateTime(state.player_mesh_animation, animation_speed, blend_rate, state.dt);
  Animation::UpdatePose(state.player_mesh_animation);
  Animation::UpdateBoneMatrices(state.player_mesh_animation);
}

// @todo debug mode only
// @todo factor to allow any skeleton pose to be visualized
static void ShowDebugPlayerSkeleton(Tachyon* tachyon, State& state, Quaternion& player_rotation, tMat4f& player_rotation_matrix) {
  profile("ShowDebugPlayerSkeleton()");

  auto& camera = tachyon->scene.camera;
  auto& meshes = state.meshes;

  tVec3f camera_to_player = state.player_position - camera.position;
  tVec3f base_position = camera.position + camera_to_player.unit() * 650.f;

  reset_instances(meshes.debug_skeleton_bone);

  auto& skeleton = state.player_mesh_animation.active_pose;

  for (auto& bone : skeleton.bones) {
    // End on the root bone, since it does not need to be visualized
    if (bone.index == skeleton.bones.size() - 1) break;

    tVec3f bone_translation = bone.translation;
    Quaternion bone_rotation = bone.rotation;
    int32 next_parent_index = bone.parent_bone_index;

    // Offset the bone so that it can be represented as a stick
    // in between its translation and the next bone, and not just
    // a point at its initial translation coordinate
    //
    // @bug this is still slightly wrong sometimes
    bone_translation += bone.rotation.toMatrix4f() * tVec3f(0, 0.15f, 0);

    auto& debug_bone = use_instance(meshes.debug_skeleton_bone);
    // @todo compute properly
    float bone_length = 10.f;

    debug_bone.position = base_position + player_rotation_matrix * (bone_translation * tVec3f(75.f));
    debug_bone.scale = tVec3f(0.5f, bone_length, 0.5f);
    debug_bone.color = tVec4f(0.2f, 0.8f, 1.f, 1.f);
    debug_bone.rotation = player_rotation * bone_rotation;

    // Show leaf bones in a different color
    if (bone.child_bone_indexes.size() == 0) {
      debug_bone.color = tVec4f(1.f, 0.2f, 1.f, 1.f);
    }

    commit(debug_bone);
  }
}

static void HandleAutoHop(State& state) {
  float jump_height = state.current_ground_y + 500.f;
  float alpha = 10.f * state.dt;

  state.player_position.y = Tachyon_Lerpf(state.player_position.y, jump_height, alpha);
}

static void HandleRunOscillation(Tachyon* tachyon, State& state, tVec3f& body_position) {
  if (
    is_key_held(tKey::CONTROLLER_A) &&
    state.player_velocity.magnitude() > 500.f
  ) {
    // Pick up run oscillation with speed
    state.run_oscillation += 5.f * state.dt;
  } else {
    // Reduce run oscillation as we slow down
    state.run_oscillation -= 3.f * state.dt;
  }

  if (state.run_oscillation < 0.f) state.run_oscillation = 0.f;
  if (state.run_oscillation > 1.f) state.run_oscillation = 1.f;

  float run_bounce_height = 250.f * state.run_oscillation;
  float run_cycle_time = 2.f * t_TAU * (fmodf(state.player_mesh_animation.seek_time + 1.f, 8.f) / 8.f) + t_HALF_PI;
  float run_bounce_cycle = sqrtf(0.5f + 0.5f * sinf(run_cycle_time));

  body_position.y += run_bounce_height * run_bounce_cycle;
}

static void HandleCombatJumpMotions(Tachyon* tachyon, State& state, tVec3f& body_position) {
  float time_since_last_dodge = time_since(state.last_dodge_time);
  float time_since_last_target_jump = time_since(state.last_target_jump_time);

  if (state.last_dodge_time != 0.f && time_since_last_dodge < 0.25f) {
    float alpha = time_since_last_dodge / 0.25f;

    body_position.y += 200.f * sinf(alpha * t_PI);
  }

  if (state.last_target_jump_time != 0.f && time_since_last_target_jump < 0.3f) {
    float alpha = time_since_last_target_jump / 0.3f;

    body_position.y += 600.f * sinf(alpha * t_PI);
  }
}

// @todo refactor to allow NPCs/enemies to turn their own heads
static bool TurnPlayerHeadTowardEntity(State& state, const GameEntity& entity, const float facing_angle) {
  tVec3f player_to_entity = entity.visible_position - state.player_position;
  float entity_direction_angle = atan2f(player_to_entity.z, player_to_entity.x);
  float turn = GetAngleBetween(entity_direction_angle, facing_angle);

  if (abs(turn) > 1.8f) {
    return false;
  }

  if (turn < -1.6f) turn = -1.6f;
  if (turn > 1.6f) turn = 1.6f;

  float& turn_angle = state.player_mesh_animation.head_turn_angle;

  turn_angle = Tachyon_Lerpf(turn_angle, -turn, 5.f * state.dt);

  return true;
}

static void TurnPlayerHeadToward(State& state, const std::vector<GameEntity>& entities, const float facing_angle) {
  for (auto& entity : entities) {
    if (!IsDuringActiveTime(entity, state)) continue;

    float entity_distance = tVec3f::distance(state.player_position, entity.visible_position);

    if (entity_distance > 7500.f) continue;

    if (TurnPlayerHeadTowardEntity(state, entity, facing_angle)) {
      break;
    }
  }
}

static void UpdatePlayerHeadTurnAngle(Tachyon* tachyon, State& state) {
  float player_facing_angle = atan2f(state.player_facing_direction.z, state.player_facing_direction.x);

  if (state.has_target) {
    // Turn head toward active target
    auto& entity = *EntityManager::FindEntity(state, state.target_entity);

    TurnPlayerHeadTowardEntity(state, entity, player_facing_angle);
  }
  else if (state.preview_target_entity_record.type != UNSPECIFIED) {
    // Turn head toward preview target
    auto& entity = *EntityManager::FindEntity(state, state.preview_target_entity_record);

    TurnPlayerHeadTowardEntity(state, entity, player_facing_angle);
  }
  else {
    // Turn head toward key entities when not targeting anything
    TurnPlayerHeadToward(state, state.sculpture_1s, player_facing_angle);
    TurnPlayerHeadToward(state, state.npcs, player_facing_angle);
    TurnPlayerHeadToward(state, state.low_guards, player_facing_angle);
    TurnPlayerHeadToward(state, state.lesser_guards, player_facing_angle);
    TurnPlayerHeadToward(state, state.wind_chimes, player_facing_angle);
  }

  // Turn head when we do quick turns, or based on tilt,
  // for slightly more natural movement when changing direction
  {
    if (time_since(state.last_quick_turn_time) < 1.f) {
      // Diminish the effect with velocity; otherwise when we
      // quick turn while running, the head takes unnaturally long
      // to rotate back into a normal forward orientation
      float alpha = 1.f - 0.75f * (state.player_velocity.magnitude() / PlayerCharacter::MAX_RUN_SPEED);

      state.player_mesh_animation.head_turn_angle += 40.f * alpha * state.tilt_angle * state.dt;
    } else {
      state.player_mesh_animation.head_turn_angle += 10.f * state.tilt_angle * state.dt;
    }
  }

  // Continually drift back toward 0
  float& turn_angle = state.player_mesh_animation.head_turn_angle;

  turn_angle = Tachyon_Lerpf(turn_angle, 0.f, 4.f * state.dt);
}

static void UpdatePlayerModel(Tachyon* tachyon, State& state, Quaternion& player_rotation, tMat4f& player_rotation_matrix) {
  auto& meshes = state.meshes;

  // Auto-hop actions
  {
    if (
      state.last_auto_hop_time != 0.f &&
      time_since(state.last_auto_hop_time) < AUTO_HOP_DURATION
    ) {
      HandleAutoHop(state);
    }
  }

  tVec3f body_scale = tVec3f(1500.f);
  Quaternion body_rotation = player_rotation;
  tVec3f body_position = state.player_position;

  if (state.player_hp > 0.f) {
    HandleRunOscillation(tachyon, state, body_position);
    HandleCombatJumpMotions(tachyon, state, body_position);
  } else {
    // Player death
    float death_alpha = 2.f * time_since(state.last_death_time);
    if (death_alpha > 1.f) death_alpha = 1.f;

    // @temporary
    tVec3f death_rotation_axis = tVec3f(1.f, 0, 0);
    Quaternion death_rotation = player_rotation * Quaternion::fromAxisAngle(death_rotation_axis, -t_HALF_PI);

    state.player_position += state.player_velocity * (1.f - death_alpha) * state.dt;
    state.player_position.y = Tachyon_Lerpf(state.player_position.y, state.current_ground_y - 1100.f, death_alpha);

    body_position = state.player_position;
    body_rotation = Quaternion::slerp(body_rotation, death_rotation, death_alpha);
  }

  UpdatePlayerSkeleton(tachyon, state);

  auto& active_pose = state.player_mesh_animation.active_pose;

  // Head
  {
    auto& head = objects(meshes.player_head)[0];
    auto& head_bone = active_pose.bones[0];

    head.position = body_position + body_rotation.toMatrix4f() * (head_bone.translation * 1200.f);
    head.rotation = body_rotation * head_bone.rotation;
    // @hack @todo fix the head model size
    head.scale = tVec3f(1300.f);
    head.color = tVec3f(0, 0, 0.1f);
    head.material = tVec4f(1.f, 0, 0, 0);

    commit(head);
  }

  // Clothing
  {
    auto& hood = skinned_mesh(meshes.player_hood);
    auto& robes = skinned_mesh(meshes.player_robes);
    auto& trim = skinned_mesh(meshes.player_trim);
    auto& shirt = skinned_mesh(meshes.player_shirt);
    auto& pants = skinned_mesh(meshes.player_pants);
    auto& boots = skinned_mesh(meshes.player_boots);
    auto& belt = skinned_mesh(meshes.player_belt);

    auto& head_bone = active_pose.bones[0];

    hood.position = body_position;
    hood.rotation = body_rotation;
    hood.scale = body_scale;
    hood.color = tVec3f(0.1f, 0.2f, 0.6f);
    hood.material = tVec4f(1.f, 0, 0, 0.2f);
    hood.shadow_cascade_ceiling = 2;
    hood.current_pose = &active_pose;

    robes.position = body_position;
    robes.rotation = body_rotation;
    robes.scale = body_scale;
    robes.color = tVec3f(0.3f, 0.4f, 0.6f);
    robes.material = tVec4f(1.f, 0, 0, 0.2f);
    robes.shadow_cascade_ceiling = 2;
    robes.current_pose = &active_pose;

    trim.position = body_position;
    trim.rotation = body_rotation;
    trim.scale = body_scale;
    trim.color = tVec3f(0.6, 0.4f, 0.2f);
    trim.material = tVec4f(0.3f, 1.f, 0, 0);
    trim.shadow_cascade_ceiling = 0;
    trim.current_pose = &active_pose;

    shirt.position = body_position;
    shirt.rotation = body_rotation;
    shirt.scale = body_scale;
    shirt.color = tVec3f(0.3f, 0.1f, 0.3f);
    shirt.material = tVec4f(1.f, 0, 0, 0.1f);
    shirt.shadow_cascade_ceiling = 2;
    shirt.current_pose = &active_pose;

    pants.position = body_position;
    pants.rotation = body_rotation;
    pants.scale = body_scale;
    pants.color = tVec3f(0.4f, 0.2f, 0.1f);
    pants.material = tVec4f(1.f, 0, 0, 0.1f);
    pants.shadow_cascade_ceiling = 2;
    pants.current_pose = &active_pose;

    boots.position = body_position;
    boots.rotation = body_rotation;
    boots.scale = body_scale;
    boots.color = tVec3f(0.1f, 0.1f, 0.1f);
    boots.material = tVec4f(1.f, 0, 0, 0);
    boots.shadow_cascade_ceiling = 2;
    boots.current_pose = &active_pose;

    belt.position = body_position;
    belt.rotation = body_rotation;
    belt.scale = body_scale;
    belt.color = tVec3f(0.4f, 0.2f, 0.1f);
    belt.material = tVec4f(0.8f, 0, 0, 0.2f);
    belt.shadow_cascade_ceiling = 0;
    belt.current_pose = &active_pose;

    commit(hood);
    commit(robes);
    commit(trim);
    commit(shirt);
    commit(pants);
    commit(boots);
    commit(belt);
  }

  if (state.show_game_stats) {
    ShowDebugPlayerSkeleton(tachyon, state, player_rotation, player_rotation_matrix);
  } else {
    reset_instances(meshes.debug_skeleton_bone);
  }
}

static void HandleWandStrike(Tachyon* tachyon, State& state) {
  Combat::HandleWandStrikeWindow(tachyon, state);
  // @deprecated
  Magic::HandleWandAction(tachyon, state);
}

static void UpdateWand(Tachyon* tachyon, State& state, Quaternion& player_rotation, tMat4f& player_rotation_matrix) {
  auto& active_pose = state.player_mesh_animation.active_pose;
  auto& wand = objects(state.meshes.player_wand)[0];
  tVec3f player_body_position = skinned_mesh(state.meshes.player_robes).position;

  tVec3f offset = player_rotation_matrix * tVec3f(-1.f, 0, 0);

  wand.scale = tVec3f(800.f);
  wand.color = tVec3f(1.f, 0.6f, 0.2f);
  wand.material = tVec4f(1.f, 0, 0, 0.4f);

  // @todo declare a constant for 5
  auto& right_hand = active_pose.bones[5];

  Quaternion held_wand_rotation = (
    right_hand.rotation *
    Quaternion::fromAxisAngle(tVec3f(0, 0, 1.f), 0.2f) *
    Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), 1.8f) *
    Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), t_PI)
  );

  if (state.wand_hold_factor > 0.f) {
    float swing_alpha = 0.002f * state.movement_distance + 2.5f * get_scene_time();
    float adjusted_pitch = 0.6f + 0.1f * sinf(swing_alpha);

    Quaternion adjusted_rotation = held_wand_rotation * (
      // Pitch correction
      Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), adjusted_pitch) *
      // Yaw correction
      Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), 0.25f) *
      // Roll correction
      Quaternion::fromAxisAngle(tVec3f(0, 0, 1.f), 0.6f)
    );

    held_wand_rotation = Quaternion::slerp(held_wand_rotation, adjusted_rotation, state.wand_hold_factor);
  }

  if (state.player_hp > 0.f) {
    // Sync the wand to the player's right hand
    // @todo factor
    auto& right_hand = active_pose.bones[5];
    tVec3f position = right_hand.translation;
    position += right_hand.rotation.toMatrix4f() * tVec3f(-0.05f, 0.32f, 0);

    wand.rotation = player_rotation * held_wand_rotation;
    wand.position = player_body_position + player_rotation_matrix * (position * 1500.f);
    wand.position -= wand.rotation.toMatrix4f() * tVec3f(0, 200.f, 0);
  }

  // Stun spell actions
  {
    if (
      state.spells.stun_start_time != 0.f &&
      time_since(state.spells.stun_start_time) < 3.f
    ) {
      float alpha = time_since(state.spells.stun_start_time) / 3.f;

      wand.position.y += sinf(alpha * t_PI) * 1200.f;
    }
  }

  // Swing actions
  {
    float time_since_last_swing = time_since(state.last_wand_swing_time);

    if (
      state.last_wand_swing_time != 0.f &&
      time_since_last_swing <= 1.2f &&
      state.player_hp > 0.f
    ) {
      tVec3f player_right = tVec3f::cross(state.player_facing_direction, tVec3f(0, 1.f, 0));

      // Draw wand up
      AnimationStep s1;
      s1.duration = ATTACK_WIND_UP_DURATION;
      s1.offset = tVec3f(0.f);
      s1.rotation = held_wand_rotation;

      // Swing the wand
      AnimationStep s2;
      s2.duration = ATTACK_SWING_DURATION;
      s2.offset = tVec3f(0, 1500.f, 0) + player_right * 500.f;
      s2.rotation = held_wand_rotation * (
        Quaternion::fromAxisAngle(tVec3f(0, 0, 1.f), t_PI) *
        Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), 1.f) *
        Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), 0.5f)
      );

      // Wind-down
      AnimationStep s3;
      s3.duration = ATTACK_WIND_DOWN_DURATION;
      s3.offset = state.player_facing_direction * 1000.f - player_right * 1000.f;
      s3.rotation = held_wand_rotation * (
        Quaternion::fromAxisAngle(tVec3f(0, 0, 1.f), t_PI) *
        Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), 1.f)
      );;

      AnimationStep s4 = s1;
      s4.rotation = held_wand_rotation;

      AnimationSequence swing_animation;
      swing_animation.steps = { s1, s2, s3, s4 };

      // Sample the animation
      TransformState sample = SimpleAnimation::Sample(swing_animation, time_since_last_swing);
      // wand.position += sample.offset;
      wand.rotation = player_rotation * sample.rotation;

      if (
        time_since_last_swing > s1.duration &&
        time_since_last_swing < s1.duration + s2.duration
      ) {
        HandleWandStrike(tachyon, state);
      }
    }

    if (
      state.last_wand_bounce_time != 0.f
    ) {
      float time_since_bounce = time_since(state.last_wand_bounce_time);
      tVec3f player_right = tVec3f::cross(state.player_facing_direction, tVec3f(0, 1.f, 0));

      // Define the animation steps
      AnimationStep s1;
      s1.duration = 0.3f;
      s1.offset = state.player_facing_direction * 1000.f - player_right * 1000.f;
      s1.rotation = held_wand_rotation;

      AnimationStep s2;
      s2.duration = 0.5f;
      s2.offset = tVec3f(0, 1500.f, 0) + player_right * 500.f;
      s2.rotation = (
        Quaternion::fromAxisAngle(tVec3f(0, 0, 1.f), 1.f) *
        Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), -1.f)
      );

      AnimationStep s3;
      s3.offset = tVec3f(0.f);
      s3.rotation = held_wand_rotation;

      AnimationSequence bounce_animation;
      bounce_animation.steps = { s1, s2, s3 };

      if (time_since_bounce > s1.duration + s2.duration) {
        // Animation complete
        state.last_wand_bounce_time = 0.f;
      } else {
        // Sample and apply the animation
        TransformState sample = SimpleAnimation::Sample(bounce_animation, time_since_bounce);
        wand.position += sample.offset;
        wand.rotation = player_rotation * sample.rotation;
      }
    }
  }

  // Player death
  {
    if (state.player_hp <= 0.f) {
      Quaternion death_rotation = player_rotation * (
        Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), 0.8f) *
        Quaternion::fromAxisAngle(tVec3f(0, 0, 1.f), -t_HALF_PI) *
        Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), -t_HALF_PI)
      );

      wand.rotation = Quaternion::slerp(wand.rotation, death_rotation, 5.f * state.dt);
      wand.position.y = Tachyon_Lerpf(wand.position.y, -1400.f, 5.f * state.dt);

      float death_alpha = 2.f * time_since(state.last_death_time);
      if (death_alpha > 1.f) death_alpha = 1.f;

      wand.position += state.player_velocity * (1.f - death_alpha) * state.dt;
    }
  }

  // Gradually bring wand sense down to zero.
  // If we're near any wand-interactible entities,
  // wand sense will increase accordingly.
  {
    state.wand_sense_factor = Tachyon_Lerpf(state.wand_sense_factor, 0.f, state.dt);
  }

  commit(wand);
}

static void UpdateWandLights(Tachyon* tachyon, State& state) {
  // Initialization
  {
    if (state.wand_lights.size() == 0) {
      for_range(0, 10) {
        WandLight wand_light;
        wand_light.light_id = create_point_light();

        state.wand_lights.push_back(wand_light);
      }
    }
  }

  float scene_time = get_scene_time();
  auto& wand = objects(state.meshes.player_wand)[0];
  tVec3f wand_end_offset = tVec3f(0, 1.48f * wand.scale.y, 0.18f * wand.scale.z);
  tVec3f wand_end_position = wand.position + wand.rotation.toMatrix4f() * wand_end_offset;
  float time_since_last_wand_swing = time_since(state.last_wand_swing_time);
  float time_since_last_wand_strike = time_since(state.last_wand_strike_time);
  tVec3f light_color = tVec3f(1.f, 0.6f, 0.2f);

  // When striking an enemy, glow blue for a short duration
  if (
    state.last_wand_strike_time != 0.f &&
    time_since_last_wand_strike < 2.f
  ) {
    const tVec3f spark_color = tVec3f(0.1f, 0.2f, 1.f);
    float alpha = time_since(state.last_wand_strike_time) / 2.f;

    light_color = tVec3f::lerp(spark_color, light_color, alpha);
  }

  // Main wand light
  {
    const float base_power = 0.1f;
    const float oscillating_power = 0.2f;
    const float wand_swing_power = 2.f;

    float oscillating_alpha = 0.5f + 0.5f * sinf(2.f * scene_time);

    auto& main_light = *get_point_light(state.wand_lights[0].light_id);
    float main_light_power = base_power + oscillating_power * oscillating_alpha;

    // Glow when swinging
    if (state.last_wand_swing_time != 0.f && time_since_last_wand_swing < 1.f) {
      float alpha = time_since_last_wand_swing;

      if (alpha < 0.1f) {
        alpha = 10.f * alpha;
      } else {
        alpha = 1.f - alpha;
      }

      main_light_power += wand_swing_power * alpha;
    }

    // Glow when holding up the wand
    {
      main_light_power += 5.f * state.wand_hold_factor;
      main_light_power += 2.f * state.wand_hold_factor * oscillating_alpha;
    }

    // Glow when close to interactibles ("wand sense")
    {
      float alpha = state.wand_sense_factor * (1.f - state.wand_hold_factor);

      main_light_power += 4.f * alpha;
      light_color = tVec3f::lerp(light_color, tVec3f(1.f), alpha);
    }

    // Glow during wind chimes actions
    if (
      state.last_wind_chimes_action_time != 0.f &&
      time_since(state.last_wind_chimes_action_time) < 4.f
    ) {
      float alpha = time_since(state.last_wind_chimes_action_time) / 4.f;
      float intensity = sinf(t_PI * alpha);

      main_light_power += 3.f * intensity;
      light_color = tVec3f::lerp(light_color, tVec3f(1.f, 0.8f, 0.6f), intensity);
    }

    // Glow during wand strikes from combat
    if (
      state.last_wand_strike_time != 0.f &&
      time_since(state.last_wand_strike_time) < 2.f
    ) {
      const tVec3f spark_color = tVec3f(0.1f, 0.2f, 1.f);
      float alpha = time_since(state.last_wand_strike_time) / 2.f;

      main_light_power += 3.f * (1.f - alpha);
    }

    main_light.position = wand_end_position;
    main_light.color = light_color;
    main_light.radius = 500.f + main_light_power * 500.f;
    main_light.power = main_light_power;
  }

  // Trailing wand lights
  {
    const float glow_duration = 1.f;
    bool spawn_new_lights = false;

    if (
      (state.last_wand_swing_time != 0.f && time_since_last_wand_swing < 0.75f) ||
      state.previous_move_delta > 5.f
    ) {
      spawn_new_lights = true;
    }

    // Spawn in new wand lights, rotating between the preallocated set
    float time_since_last_wand_light = time_since(state.last_wand_light_time);

    for_range(1, 10) {
      auto& wand_light = state.wand_lights[i];
      auto& light = *get_point_light(wand_light.light_id);

      if (
        spawn_new_lights &&
        time_since(wand_light.spawn_time) > glow_duration &&
        time_since_last_wand_light > 0.1f
      ) {
        state.last_wand_light_time = scene_time;

        wand_light.spawn_time = scene_time;
        light.position = wand_end_position;

        break;
      }

      float power_alpha = time_since(wand_light.spawn_time) / glow_duration;
      if (power_alpha > 1.f) power_alpha = 1.f;

      float power = sinf(power_alpha * t_PI);

      light.color = light_color;
      light.radius = 500.f;
      light.glow_power = power;
      light.power = power;
    }
  }
}

static void UpdateLantern(Tachyon* tachyon, State& state, const Quaternion& player_rotation, const tMat4f& player_rotation_matrix) {
  // Lantern object
  {
    // @temporary
    static float lantern_swing = 0.f;

    float speed_ratio = state.previous_move_delta / 90.f;

    lantern_swing += state.dt * speed_ratio;
    lantern_swing *= 1.f - state.dt;

    if (lantern_swing < 0.f) lantern_swing = 0.f;
    if (lantern_swing > 1.f) lantern_swing = 1.f;

    float lantern_angle = lantern_swing * sinf(get_scene_time() * 10.f);
    Quaternion lantern_rotation = Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), lantern_angle);

    auto& lantern = objects(state.meshes.player_lantern)[0];

    tVec3f offset = player_rotation_matrix * tVec3f(
      525.f,
      -100.f + 200.f * abs(lantern_angle),
      -100.f * lantern_angle
    );

    lantern.position = state.player_position + offset;
    lantern.scale = tVec3f(80.f, 120.f, 80.f);
    lantern.rotation = player_rotation * lantern_rotation;
    lantern.color = state.is_nighttime ? tVec4f(1.f, 0.7f, 0.3f, 1.f) : tVec4f(0.8f, 0.6f, 0.4f, 1.f);
    lantern.material = tVec4f(1.f, 0, 0, 1.f);

    commit(lantern);
  }

  // Point light source
  {
    auto& light = *get_point_light(state.player_light_id);

    tVec3f offset = player_rotation_matrix * tVec3f(710.f, 50.f, 0);

    light.position = state.player_position + offset;
    light.position.y -= 300.f;
    light.radius = state.is_nighttime ? 4000.f : 2500.f;
    light.color = tVec3f(0.5f, 0.3f, 0.6f);
    light.color = get_point_light(state.astrolabe_light_id)->color;
    light.power = state.is_nighttime ? 1.f : 0.5f;
    light.glow_power = 0.f;

    // @todo factor (Astrolabe::)
    if (time_since(state.game_time_at_start_of_turn) < 2.f) {
      float alpha = time_since(state.game_time_at_start_of_turn) / 2.f;

      light.power += sinf(alpha * t_PI);
    }
  }
}

void PlayerCharacter::UpdatePlayer(Tachyon* tachyon, State& state) {
  profile("UpdatePlayer()");

  // Update facing direction and tilt
  // @todo factor
  {
    float player_speed = state.player_velocity.magnitude();
    float speed_ratio = player_speed / PlayerCharacter::MAX_RUN_SPEED;
    tVec3f desired_facing_direction = state.player_facing_direction;
    float turn_speed = Tachyon_Lerpf(2.f, 10.f, speed_ratio);
    float tilt = 0.f;

    if (state.has_target) {
      // When we're focused on a target, continue to face toward it
      auto& target = *EntityManager::FindEntity(state, state.target_entity);

      desired_facing_direction = (target.visible_position - state.player_position).xz().unit();
    }
    else if (state.player_hp > 0.f && player_speed > 0.01f) {
      // Without a target, use our velocity vector to influence facing direction
      desired_facing_direction = state.player_velocity.unit();
    }

    // When astro turning, don't change our facing direction at all,
    // since targeted entities may jitter and jump about rapidly,
    // and we don't want the facing direction being thrown off
    //
    // @todo remove this since we detarget things upon astro traveling now
    if (abs(state.astro_turn_speed) > 0.05f) {
      turn_speed = 0.f;
    }

    // Calculate tilt before applying the new facing direction
    float facing_angle = atan2f(state.player_facing_direction.z, state.player_facing_direction.x);
    float desired_facing_angle = atan2f(desired_facing_direction.z, desired_facing_direction.x);

    tilt = GetAngleBetween(desired_facing_angle, facing_angle);
    tilt *= 0.3f;
    tilt *= speed_ratio;

    state.player_facing_direction = tVec3f::slerp(state.player_facing_direction, desired_facing_direction, turn_speed * state.dt).unit();
    state.tilt_angle = Tachyon_Lerpf(state.tilt_angle, tilt, 5.f * state.dt);

    state.player_mesh_animation.torso_turn_angle = 4.f * state.tilt_angle;
  }

  Quaternion player_rotation = (
    // Rotate according to facing direction
    Quaternion::FromDirection(state.player_facing_direction, tVec3f(0, 1.f, 0)) *
    // Apply tilt
    Quaternion::fromAxisAngle(tVec3f(0, 0, 1.f), state.tilt_angle)
  );

  tMat4f player_rotation_matrix = player_rotation.toMatrix4f();

  UpdatePlayerHeadTurnAngle(tachyon, state);
  UpdatePlayerModel(tachyon, state, player_rotation, player_rotation_matrix);
  UpdateLantern(tachyon, state, player_rotation, player_rotation_matrix);

  if (Items::HasItem(state, MAGIC_WAND)) {
    UpdateWand(tachyon, state, player_rotation, player_rotation_matrix);
    UpdateWandLights(tachyon, state);
  }
}

void PlayerCharacter::AutoHop(Tachyon* tachyon, State& state) {
  if (time_since(state.last_auto_hop_time) > AUTO_HOP_DURATION) {
    state.last_auto_hop_time = get_scene_time();
  }
}

bool PlayerCharacter::CanTakeDamage(Tachyon* tachyon, const State& state) {
  return (
    time_since(state.last_damage_time) > 1.5f &&
    time_since(state.last_target_jump_time) > 1.f &&
    time_since(state.last_dodge_time) > 0.3f &&
    time_since(state.last_break_attack_time) > 1.f
  );
}

bool PlayerCharacter::IsRunning(Tachyon* tachyon, State& state) {
  return (
    is_key_held(tKey::CONTROLLER_A) &&
    state.previous_move_delta > 0.f &&
    (abs(tachyon->left_stick.x) > 0.1f || abs(tachyon->left_stick.y) > 0.1f)
  );
}

float PlayerCharacter::GetHumanEnemyAlertedSpeed(const State& state) {
  float max_walking_speed =
    state.has_target
      ? PlayerCharacter::MAX_COMBAT_WALK_SPEED
      : PlayerCharacter::MAX_WALK_SPEED;

  return max_walking_speed - 1.f;
}

void PlayerCharacter::TakeDamage(Tachyon* tachyon, State& state, const float damage) {
  state.player_hp -= damage;
  state.last_damage_time = get_scene_time();

  // Cancel any dodge motions
  state.last_dodge_time = 0.f;
  state.last_target_jump_time = 0.f;

  // Cancel attack animation
  state.last_wand_swing_time = 0.f;
  state.player_mesh_animation.upper_body_animation = nullptr;
  state.player_mesh_animation.upper_body_animation_time = 0.f;

  if (state.player_hp <= 0.f) {
    // @temporary
    UISystem::ShowBlockingDialogue(tachyon, state, "YOU DIED");

    state.last_death_time = get_scene_time();
    state.last_wand_swing_time = 0.f;
    state.last_wand_bounce_time = 0.f;
  }
}

void PlayerCharacter::GetKnockedBack(State& state, float speed) {
  state.player_velocity = state.player_facing_direction.invert() * speed;
}

void PlayerCharacter::PerformStandardDodgeAction(Tachyon* tachyon, State& state) {
  state.player_velocity = state.player_velocity.unit() * 3400.f;
  state.last_dodge_time = get_scene_time();
  state.last_break_attack_time = 0.f;
  state.last_target_jump_time = 0.f;

  Sfx::PlaySound(SFX_DODGE, 0.5f);
}

void PlayerCharacter::PerformTargetJumpAction(Tachyon* tachyon, State& state) {
  auto& target = *EntityManager::FindEntity(state, state.target_entity);
  float target_distance = tVec3f::distance(target.visible_position, state.player_position);
  tVec3f target_direction = (target.visible_position - state.player_position) / target_distance;

  state.player_velocity = target_direction * target_distance * 0.4f;
  state.last_target_jump_time = get_scene_time();
  state.last_dodge_time = 0.f;
  state.last_break_attack_time = 0.f;

  // @todo replace
  Sfx::PlaySound(SFX_DODGE, 0.5f);
}