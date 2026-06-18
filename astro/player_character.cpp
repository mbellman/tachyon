#include "astro/player_character.h"
#include "astro/animation.h"
#include "astro/astrolabe.h"
#include "astro/combat.h"
#include "astro/entity_behaviors/behavior.h"
#include "astro/entity_manager.h"
#include "astro/items.h"
#include "astro/magic.h"
#include "astro/player_animation.h"
#include "astro/player_attachments.h"
#include "astro/player_wand.h"
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

constexpr static float RUN_BOUNCE_HEIGHT = 275.f;

static std::vector<float> run_bounce_curve = {
  0.1f,
  0.6f,
  0.9f,
  1.f,
  0.8f,
  0.4f,
  -0.3f,
  -0.2f
};

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

static bool IsWandHoldAnimationActive(State& state) {
  return HasCurrentWandAnimation(state) || HasNextWandAnimation(state);
}

// @todo debug mode only
// @todo factor to allow any skeleton pose to be visualized
static void ShowDebugPlayerSkeleton(Tachyon* tachyon, State& state) {
  profile("ShowDebugPlayerSkeleton()");

  auto& camera = tachyon->scene.camera;
  auto& meshes = state.meshes;

  tVec3f camera_to_player = state.player.visual_position - camera.position;
  tVec3f base_position = camera.position + camera_to_player.unit() * 650.f;

  reset_instances(meshes.debug_skeleton_bone);

  auto& skeleton = state.player.rig.active_pose;

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

    debug_bone.position = base_position + state.player.rotation_matrix * (bone_translation * tVec3f(75.f));
    debug_bone.scale = tVec3f(0.5f, bone_length, 0.5f);
    debug_bone.color = tVec4f(0.2f, 0.8f, 1.f, 1.f);
    debug_bone.rotation = state.player.rotation * bone_rotation;

    // Show leaf bones in a different color
    if (bone.child_bone_indexes.size() == 0) {
      debug_bone.color = tVec4f(1.f, 0.2f, 1.f, 1.f);
    }

    commit(debug_bone);
  }
}

static void TrackPlantedFootPositionWhileRunning(Tachyon* tachyon, State& state) {
  auto& rig = state.player.rig;
  float t = fmodf(rig.seek_time, 8.f);

  // Airborne
  if (!state.player.is_airborne_in_run_cycle && t > 7.f) {
    state.player.is_airborne_in_run_cycle = true;
    state.player.is_left_foot_planted = false;
    state.player.is_right_foot_planted = false;
  }

  // Airborne
  if (!state.player.is_airborne_in_run_cycle && t > 3.f && t < 5.5f) {
    state.player.is_airborne_in_run_cycle = true;
    state.player.is_left_foot_planted = false;
    state.player.is_right_foot_planted = false;
  }

  // Planted (left foot)
  if (state.player.is_airborne_in_run_cycle && t > 1.5f && t < 3.f) {
    state.player.is_airborne_in_run_cycle = false;
    state.player.is_left_foot_planted = true;
    state.player.is_right_foot_planted = false;

    tVec3f foot = rig.active_pose.bones[9].translation * 1500.f;
    foot = state.player.rotation_matrix * foot;

    state.player.planted_left_foot_position = state.player_position + foot;
  }

  // Planted (right foot)
  if (state.player.is_airborne_in_run_cycle && t > 5.5f && t < 7.f) {
    state.player.is_airborne_in_run_cycle = false;
    state.player.is_left_foot_planted = false;
    state.player.is_right_foot_planted = true;

    tVec3f foot = rig.active_pose.bones[13].translation * 1500.f;
    foot = state.player.rotation_matrix * foot;

    state.player.planted_right_foot_position = state.player_position + foot;
  }
}

static void TrackPlantedFootPositionsWhileWalking(Tachyon* tachyon, State& state) {
  auto& rig = state.player.rig;
  float t = fmodf(rig.seek_time, 8.f);

  // Airborne
  if (!state.player.is_airborne_in_run_cycle && t > 0.f && t < 1.f) {
    state.player.is_airborne_in_run_cycle = true;
    state.player.is_left_foot_planted = false;
    state.player.is_right_foot_planted = false;
  }

  // Airborne
  if (!state.player.is_airborne_in_run_cycle && t > 4.f && t < 5.f) {
    state.player.is_airborne_in_run_cycle = true;
    state.player.is_left_foot_planted = false;
    state.player.is_right_foot_planted = false;
  }

  // Planted (left foot)
  if (state.player.is_airborne_in_run_cycle && t > 1.f && t < 4.f) {
    state.player.is_airborne_in_run_cycle = false;
    state.player.is_left_foot_planted = true;
    state.player.is_right_foot_planted = false;

    tVec3f foot = rig.active_pose.bones[9].translation * 1500.f;
    foot = state.player.rotation_matrix * foot;

    state.player.planted_left_foot_position = state.player_position + foot;
  }

  // Planted (right foot)
  if (state.player.is_airborne_in_run_cycle && t > 5.f) {
    state.player.is_airborne_in_run_cycle = false;
    state.player.is_left_foot_planted = false;
    state.player.is_right_foot_planted = true;

    tVec3f foot = rig.active_pose.bones[13].translation * 1500.f;
    foot = state.player.rotation_matrix * foot;

    state.player.planted_right_foot_position = state.player_position + foot;
  }
}

// @todo move elsewhere
static float SampleCurve(const std::vector<float>& curve, const float t) {
  float max_time = float(curve.size());
  float seek_time = t * max_time;
  if (seek_time < 0.f) seek_time += max_time;

  int start_frame = (int) seek_time;
  int end_frame = start_frame + 1;

  float a = curve[start_frame % 8];
  float b = curve[end_frame % 8];
  float alpha = fmodf(seek_time, 1.f);

  return Tachyon_Lerpf(a, b, alpha);
}

static void HandleRunOscillation(Tachyon* tachyon, State& state) {
  if (state.did_jump_off_ledge) {
    // Reduce run oscillation when jumping off ledges
    state.run_oscillation -= 5.f * state.dt;
  }
  else if (
    is_key_held(tKey::CONTROLLER_A) &&
    state.player_velocity.magnitude() > 500.f
  ) {
    // Pick up run oscillation with speed
    state.run_oscillation += 5.f * state.dt;
  }
  else {
    // Reduce run oscillation as we slow down
    state.run_oscillation -= 3.f * state.dt;
  }

  if (state.run_oscillation < 0.f) state.run_oscillation = 0.f;
  if (state.run_oscillation > 1.f) state.run_oscillation = 1.f;

  float run_bounce_height = RUN_BOUNCE_HEIGHT * state.run_oscillation;
  float run_cycle_time = fmodf(state.player.rig.seek_time + 1.f, 8.f) / 8.f;
  float run_bounce = SampleCurve(run_bounce_curve, 2.f * run_cycle_time);

  state.player.visual_position.y += run_bounce_height * run_bounce;
}

static void HandleCombatJumpMotions(Tachyon* tachyon, State& state) {
  float time_since_last_dodge = time_since(state.last_dodge_time);
  float time_since_last_target_jump = time_since(state.last_target_jump_time);

  if (state.last_dodge_time != 0.f && time_since_last_dodge < 0.25f) {
    float alpha = time_since_last_dodge / 0.25f;

    state.player.visual_position.y += 200.f * sinf(alpha * t_PI);
  }

  if (state.last_target_jump_time != 0.f && time_since_last_target_jump < 0.3f) {
    float alpha = time_since_last_target_jump / 0.3f;

    state.player.visual_position.y += 600.f * sinf(alpha * t_PI);
  }
}

static void UpdatePlayerModel(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;
  auto& player = state.player;

  // Ledge jump actions
  {
    if (state.did_jump_off_ledge) {
      float time_since_jump = time_since(state.last_ledge_jump_time);

      if (time_since_jump < 0.2f) {
        float alpha = time_since_jump / 0.2f;

        state.player_position.y += 6000.f * state.dt * (1.f - alpha);
      }
    }
  }

  player.visual_position = state.player_position;
  player.visual_rotation = player.rotation;

  if (state.player_hp > 0.f) {
    if (PlayerCharacter::IsRunning(tachyon, state)) {
      TrackPlantedFootPositionWhileRunning(tachyon, state);
    } else if (
      state.previous_move_delta > 0.f &&
      !state.is_on_ladder &&
      !PlayerCharacter::IsClimbingOffLadder(tachyon, state)
    ) {
      TrackPlantedFootPositionsWhileWalking(tachyon, state);
    } else {
      state.player.is_left_foot_planted = false;
      state.player.is_right_foot_planted = false;
    }

    HandleRunOscillation(tachyon, state);
    HandleCombatJumpMotions(tachyon, state);
  } else {
    // Player death
    // @todo factor
    float death_alpha = 2.f * time_since(state.last_death_time);
    if (death_alpha > 1.f) death_alpha = 1.f;

    // @temporary
    // @todo Animation? This honestly isn't too bad for now.
    tVec3f death_rotation_axis = tVec3f(1.f, 0, 0);
    Quaternion death_rotation = player.rotation * Quaternion::fromAxisAngle(death_rotation_axis, -t_HALF_PI);

    state.player_position += state.player_velocity * (1.f - death_alpha) * state.dt;
    state.player_position.y = Tachyon_Lerpf(state.player_position.y, state.current_ground_y - 1100.f, death_alpha);

    player.visual_position = state.player_position;
    player.visual_rotation = Quaternion::slerp(player.visual_rotation, death_rotation, death_alpha);
  }

  PlayerAnimation::Update(tachyon, state);
  PlayerAttachments::Update(tachyon, state);

  // Keep feet planted
  // @todo factor
  {
    auto& rig = state.player.rig;

    if (state.player.is_left_foot_planted) {
      tVec3f foot = rig.active_pose.bones[9].translation * 1500.f;
      foot = player.rotation_matrix * foot;

      tVec3f current_left_foot_position = state.player_position + foot;
      tVec3f offset = state.player.planted_left_foot_position - current_left_foot_position;

      state.player_position += offset.xz() * 0.1f;
    }

    if (state.player.is_right_foot_planted) {
      tVec3f foot = rig.active_pose.bones[13].translation * 1500.f;
      foot = player.rotation_matrix * foot;

      tVec3f current_right_foot_position = state.player_position + foot;
      tVec3f offset = state.player.planted_right_foot_position - current_right_foot_position;

      state.player_position += offset.xz() * 0.1f;
    }
  }

  // @todo make this a constant
  tVec3f body_scale = tVec3f(1500.f);
  auto& active_pose = state.player.rig.active_pose;

  // Head
  {
    auto& head = objects(meshes.player_head)[0];
    auto& head_bone = active_pose.bones[0];

    head.position = player.visual_position + player.visual_rotation.toMatrix4f() * (head_bone.translation * 1200.f);
    head.rotation = player.visual_rotation * head_bone.rotation;
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
    auto& vambraces = skinned_mesh(meshes.player_vambraces);
    auto& trim = skinned_mesh(meshes.player_trim);
    auto& shirt = skinned_mesh(meshes.player_shirt);
    auto& pants = skinned_mesh(meshes.player_pants);
    auto& boots = skinned_mesh(meshes.player_boots);
    auto& belt = skinned_mesh(meshes.player_belt);

    hood.position = player.visual_position;
    hood.rotation = player.visual_rotation;
    hood.scale = body_scale;
    hood.color = tVec4f(0.2f, 0.3f, 0.7f, 0.2f);
    hood.material = tVec4f(1.f, 0, 0, 0.2f);
    hood.shadow_cascade_ceiling = 2;
    hood.current_pose = &active_pose;

    // @todo factor
    float speed_ratio = state.player_velocity.magnitude() / PlayerCharacter::MAX_RUN_SPEED;
    float t = fmodf(state.player.rig.seek_time + 3.f, 8.f) / 8.f;

    tVec3f flop = tVec3f(
      150.f * speed_ratio * cosf(t * t_TAU),
      250.f * speed_ratio * sinf(2.f * t * t_TAU),
      0.f
    );

    hood.flop_control_point = tVec3f(0, 1.3f, -0.5f);
    hood.flop_offset = state.player.rotation_matrix * flop;

    commit(hood);

    robes.position = player.visual_position;
    robes.rotation = player.visual_rotation;
    robes.scale = body_scale;
    robes.color = tVec4f(0.3f, 0.4f, 0.6f, 0.2f);
    robes.material = tVec4f(1.f, 0, 0, 0.2f);
    robes.shadow_cascade_ceiling = 2;
    robes.current_pose = &active_pose;

    commit(robes);

    vambraces.position = player.visual_position;
    vambraces.rotation = player.visual_rotation;
    vambraces.scale = body_scale;
    vambraces.color = 0x3220;
    vambraces.material = tVec4f(0.6f, 0, 0, 0.2f);
    vambraces.shadow_cascade_ceiling = 2;
    vambraces.current_pose = &active_pose;

    commit(vambraces);

    trim.position = player.visual_position;
    trim.rotation = player.visual_rotation;
    trim.scale = body_scale;
    trim.color = tVec3f(0.6f, 0.4f, 0.2f);
    trim.material = tVec4f(0.3f, 1.f, 0, 0);
    trim.shadow_cascade_ceiling = 0;
    trim.current_pose = &active_pose;

    commit(trim);

    shirt.position = player.visual_position;
    shirt.rotation = player.visual_rotation;
    shirt.scale = body_scale;
    shirt.color = tVec3f(0.3f, 0.1f, 0.3f);
    shirt.material = tVec4f(1.f, 0, 0, 0.1f);
    shirt.shadow_cascade_ceiling = 2;
    shirt.current_pose = &active_pose;

    commit(shirt);

    pants.position = player.visual_position;
    pants.rotation = player.visual_rotation;
    pants.scale = body_scale;
    pants.color = tVec3f(0.6f, 0.4f, 0.2f);
    pants.material = tVec4f(1.f, 0, 0, 0.1f);
    pants.shadow_cascade_ceiling = 2;
    pants.current_pose = &active_pose;

    commit(pants);

    boots.position = player.visual_position;
    boots.rotation = player.visual_rotation;
    boots.scale = body_scale;
    boots.color = tVec3f(0.1f, 0.1f, 0.1f);
    boots.material = tVec4f(1.f, 0, 0, 0);
    boots.shadow_cascade_ceiling = 2;
    boots.current_pose = &active_pose;

    commit(boots);

    belt.position = player.visual_position;
    belt.rotation = player.visual_rotation;
    belt.scale = body_scale;
    belt.color = 0x2110;
    belt.material = tVec4f(0.8f, 0, 0, 0.2f);
    belt.shadow_cascade_ceiling = 0;
    belt.current_pose = &active_pose;

    commit(belt);
  }

  if (state.show_game_stats) {
    ShowDebugPlayerSkeleton(tachyon, state);
  } else {
    reset_instances(meshes.debug_skeleton_bone);
  }
}

static void HandleWandStrike(Tachyon* tachyon, State& state) {
  Combat::HandleWandStrikeWindow(tachyon, state);
  // @deprecated
  Magic::HandleWandAction(tachyon, state);
}

static float GetWandHoldFactor(State& state) {
  bool has_current_wand_animation = HasCurrentWandAnimation(state);
  bool has_next_wand_animation = HasNextWandAnimation(state);

  return (
    !has_current_wand_animation && !has_next_wand_animation
      ? 0.f :
    !has_current_wand_animation && has_next_wand_animation
      // Taking the wand out
      ? state.player.rig.next_animation_blend_alpha :
    has_current_wand_animation && !has_next_wand_animation
      // Putting the wand down
      ? 1.f - state.player.rig.next_animation_blend_alpha :
    1.f
  );
}

static void UpdateWand(Tachyon* tachyon, State& state) {
  auto& active_pose = state.player.rig.active_pose;
  auto& wand = objects(state.meshes.player_wand)[0];
  tVec3f player_body_position = skinned_mesh(state.meshes.player_robes).position;

  tVec3f offset = state.player.rotation_matrix * tVec3f(-1.f, 0, 0);

  wand.scale = state.is_on_ladder || time_since(state.player.last_climbing_stop_time) < 0.5f ? tVec3f(0.f) : tVec3f(800.f);
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

  if (IsWandHoldAnimationActive(state)) {
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

    float alpha = GetWandHoldFactor(state);

    held_wand_rotation = Quaternion::slerp(held_wand_rotation, adjusted_rotation, alpha);
  }

  if (state.player_hp > 0.f) {
    // Sync the wand to the player's right hand
    // @todo factor
    auto& right_hand = active_pose.bones[5];
    tVec3f position = right_hand.translation;
    position += right_hand.rotation.toMatrix4f() * tVec3f(-0.f, 0.35f, 0);

    wand.rotation = state.player.rotation * held_wand_rotation;
    wand.position = player_body_position + state.player.rotation_matrix * (position * 1500.f);
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
      wand.rotation = state.player.rotation * sample.rotation;

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
        wand.rotation = state.player.rotation * sample.rotation;
      }
    }
  }

  // Player death
  {
    if (state.player_hp <= 0.f) {
      Quaternion death_rotation = state.player.rotation * (
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

  auto& fx = tachyon->fx;
  float scene_time = get_scene_time();
  auto& wand = objects(state.meshes.player_wand)[0];
  tVec3f wand_end_offset = tVec3f(0, 1.48f * wand.scale.y, 0.18f * wand.scale.z);
  tVec3f wand_end_position = wand.position + wand.rotation.toMatrix4f() * wand_end_offset;
  float time_since_last_wand_swing = time_since(state.last_wand_swing_time);
  float time_since_last_wand_strike = time_since(state.last_wand_strike_time);
  tVec3f light_color = tVec3f(1.f, 0.75f, 0.4f);

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
      float time_since_wand_pulse = time_since(state.last_wand_light_pulse_time);

      if (IsWandHoldAnimationActive(state)) {
        float wand_hold_factor = GetWandHoldFactor(state);
        float pulse_alpha = time_since_wand_pulse / 4.f;

        clamp_to_1(pulse_alpha);

        float glow;

        if (pulse_alpha < 0.2f) {
          glow = Tachyon_EaseOutSine(pulse_alpha * 5.f);
        } else {
          glow = 1.f - Tachyon_EaseInOutf((pulse_alpha - 0.2f) / 0.8f);
        }

        float glow_intensity = 5.f * glow;
        float oscillation = 0.5f * oscillating_alpha;

        main_light_power += wand_hold_factor * (glow_intensity + oscillation);
      }

      // Perform a pulse effect after a short duration
      if (
        state.last_wand_light_pulse_time != 0.f &&
        time_since_wand_pulse > 0.2f &&
        time_since_wand_pulse < 4.f
      ) {
        fx.wand_pulse_position = wand_end_position;
        fx.wand_pulse_alpha = (time_since_wand_pulse - 0.2f) / 3.8f;
      } else {
        // Setting the alpha to 1 renders the effect "complete"
        fx.wand_pulse_alpha = 1.f;
      }
    }

    // Glow when close to interactibles ("wand sense")
    {
      if (state.wand_sense_factor > 0.f) {
        float inverse_wand_hold_factor = IsWandHoldAnimationActive(state)
          ? 1.f - GetWandHoldFactor(state)
          : 1.f;

        float alpha = state.wand_sense_factor * inverse_wand_hold_factor;

        main_light_power += 4.f * alpha;
        light_color = tVec3f::lerp(light_color, tVec3f(1.f), alpha);
      }
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

    // Disable when climbing/climbing off ladders
    if (state.is_on_ladder || PlayerCharacter::IsClimbingOffLadder(tachyon, state)) {
      main_light_power = 0.f;
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
      (
        !state.is_on_ladder &&
        !PlayerCharacter::IsClimbingOffLadder(tachyon, state)
      ) && (
        (state.last_wand_swing_time != 0.f && time_since_last_wand_swing < 0.75f) ||
        state.previous_move_delta > 5.f
      )
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

void PlayerCharacter::UpdatePlayer(Tachyon* tachyon, State& state) {
  profile("UpdatePlayer()");

  float player_speed = state.player_velocity.magnitude();
  float speed_ratio = player_speed / PlayerCharacter::MAX_RUN_SPEED;

  // Update facing direction and tilt
  // @todo factor
  {
    tVec3f desired_facing_direction = state.player_facing_direction;
    float turn_speed = Tachyon_Lerpf(2.f, 10.f, speed_ratio);
    float tilt = 0.f;

    if (state.has_target) {
      // When we're focused on a target, continue to face toward it
      auto& target = *EntityManager::FindEntity(state, state.target_entity);

      desired_facing_direction = (target.visible_position - state.player_position).xz().unit();
    }
    else if (
      state.player_hp > 0.f &&
      player_speed > 0.01f &&
      !state.is_on_ladder &&
      !PlayerCharacter::IsClimbingOffLadder(tachyon, state)
    ) {
      // Without a target, use our velocity vector to influence facing direction
      desired_facing_direction = state.player_velocity.xz().unit();
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
  }

  // Set current rotation
  // @todo factor
  {
    state.player.rotation = (
      // Rotate according to facing direction
      Quaternion::FromDirection(state.player_facing_direction, tVec3f(0, 1.f, 0)) *
      // Apply tilt
      Quaternion::fromAxisAngle(tVec3f(0, 0, 1.f), state.tilt_angle)
    );

    state.player.rotation_matrix = state.player.rotation.toMatrix4f();
  }

  UpdatePlayerModel(tachyon, state);

  if (Items::HasItem(state, MAGIC_WAND)) {
    UpdateWand(tachyon, state);
    UpdateWandLights(tachyon, state);
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
    is_moving_left_stick() &&
    !state.is_on_ladder &&
    !PlayerCharacter::IsClimbingOffLadder(tachyon, state)
  );
}

bool PlayerCharacter::IsClimbingOffLadder(Tachyon* tachyon, State& state) {
  float climbing_stop_time = state.player.last_climbing_stop_time;

  if (climbing_stop_time == 0.f) {
    return false;
  }

  if (state.did_climb_down) {
    return time_since(climbing_stop_time) < 0.8f;
  } else {
    return time_since(climbing_stop_time) < 1.6f;
  }
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
  state.player.rig.upper_body_animation = nullptr;
  state.player.rig.upper_body_animation_time = 0.f;

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