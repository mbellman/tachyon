#include "astro/player_character.h"
#include "astro/combat.h"
#include "astro/entity_manager.h"
#include "astro/sfx.h"
#include "astro/simple_animation.h"
#include "astro/ui_system.h"

using namespace astro;

const static float AUTO_HOP_DURATION = 0.3f;

// @todo move to Animation::
static void ReserveAnimationPoseData(State::SkeletonAnimation& animation) {
  if (animation.current_pose.bones.size() == 0) {
    for (auto& bone : animation.frames[0].bones) {
      animation.current_pose.bones.push_back(bone);
    }
  }
}

// @todo move to Animation::
static void EvaluateAnimation(State::SkeletonAnimation& animation, const float seek_time) {
  float blend_alpha = fmodf(seek_time, 1.f);

  if (animation.frames.size() == 2) {
    // Special treatment for 2-frame animations: alternate between
    // both frames using an ease-in-out transition
    blend_alpha = Tachyon_EaseInOutf(blend_alpha);
  }

  int32 start_index = int(seek_time) % animation.frames.size();
  int32 end_index = (start_index + 1) % animation.frames.size();

  auto& start_frame = animation.frames[start_index];
  auto& end_frame = animation.frames[end_index];

  for (int32 i = 0; i < start_frame.bones.size(); i++) {
    auto& start_bone = start_frame.bones[i];
    auto& end_bone = end_frame.bones[i];
    Quaternion blended_rotation = Quaternion::nlerp(start_bone.rotation, end_bone.rotation, blend_alpha);

    animation.current_pose.bones[i].rotation = blended_rotation;

    // @todo blend translations (+ scale??)
  }
}

// @todo move to Animation::
static void SetNextAnimation(State& state, State::SkeletonAnimation* animation) {
  if (state.next_animation == animation) {
    return;
  }

  ReserveAnimationPoseData(*animation);

  state.next_animation = animation;
  state.time_since_last_animation_change = 0.f;
}

static float GetMaxSeekTime(State::SkeletonAnimation& animation) {
  return (float)animation.frames.size();
}

// @todo move to Animation::
// @todo refactor to allow for generic skeleton animation blending etc.
static void UpdatePlayerSkeleton(Tachyon* tachyon, State& state) {
  profile("UpdatePlayerSkeleton()");

  // Set the default current animation if not initialized
  if (state.current_animation == nullptr) {
    state.current_animation = &state.animations.player_idle;
  }

  // @temporary
  // @todo set the active animation based on player actions
  if (state.player_velocity.magnitude() > 600.f) {
    SetNextAnimation(state, &state.animations.player_run);
  } else if (state.player_velocity.magnitude() > 50.f) {
    SetNextAnimation(state, &state.animations.player_walk);
  } else {
    SetNextAnimation(state, &state.animations.player_idle);
  }

  // Accumulate animation time
  state.animation_seek_time += state.next_animation->speed * state.dt;

  // Limit and wrap the animation time to a common multiple of the
  // current/next animation times so that we don't eventually encounter
  // accumulation precision errors
  {
    float max_seek_time = GetMaxSeekTime(*state.current_animation) * GetMaxSeekTime(*state.next_animation);

    if (state.animation_seek_time > max_seek_time) {
      state.animation_seek_time -= max_seek_time;
    }
  }

  // Track time between changing animations so they can be blended
  {
    state.time_since_last_animation_change += 3.f * state.dt;

    if (state.time_since_last_animation_change > 1.f) {
      state.time_since_last_animation_change = 1.f;
    }
  }

  // Update the current animation when a full blend from current -> next is complete
  if (
    state.current_animation != state.next_animation &&
    state.time_since_last_animation_change == 1.f
  ) {
    state.current_animation = state.next_animation;
  }

  // Evaluate the current and next animations simultaneously so they can be blended
  // @optimize this only has to be done when transitioning between animations
  EvaluateAnimation(*state.current_animation, state.animation_seek_time);
  EvaluateAnimation(*state.next_animation, state.animation_seek_time);

  // Update the current pose based on the blended result of the current/next animations
  {
    auto& current_pose = state.player_current_pose;
    float blend_alpha = state.time_since_last_animation_change;

    for (int32 i = 0; i < state.current_animation->current_pose.bones.size(); i++) {
      auto& previous_bone = state.current_animation->current_pose.bones[i];
      auto& next_bone = state.next_animation->current_pose.bones[i];
      auto& current_pose_bone = current_pose.bones[i];
      Quaternion blended_rotation = Quaternion::nlerp(previous_bone.rotation, next_bone.rotation, blend_alpha);

      // Reset current pose bone translation back to bone space
      current_pose_bone.translation = previous_bone.translation;

      // Set blended rotation
      current_pose_bone.rotation = blended_rotation;
    }
  }

  // Compute bone matrices for use in the renderer
  // @todo factor
  {
    auto& rest_pose = state.player_rest_pose;
    auto& current_pose = state.player_current_pose;

    current_pose.bone_matrices.clear();

    for (auto& bone : state.player_current_pose.bones) {
      int32 next_parent_index = bone.parent_bone_index;

      // @todo refactor to use TransformBonesIntoMeshSpace()
      while (next_parent_index != -1) {
        auto& parent_bone = current_pose.bones[next_parent_index];

        bone.translation = parent_bone.translation + parent_bone.rotation.toMatrix4f() * bone.translation;
        bone.rotation = parent_bone.rotation * bone.rotation;

        next_parent_index = parent_bone.parent_bone_index;
      }

      tMat4f inverse_bind_matrix = rest_pose.bone_matrices[bone.index];
      tMat4f pose_matrix = tMat4f::transformation(bone.translation, tVec3f(1.f), bone.rotation);
      tMat4f bone_matrix = pose_matrix * inverse_bind_matrix;

      current_pose.bone_matrices.push_back(bone_matrix.transpose());
    }
  }
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

  auto& skeleton = state.player_current_pose;

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

static void HandleRunOscillation(State& state, tVec3f& body_position, const float player_speed) {
  if (player_speed > 600.f) {
    state.run_oscillation += 5.f * state.dt;
  } else {
    state.run_oscillation -= 4.f * state.dt;
  }

  if (state.run_oscillation < 0.f) state.run_oscillation = 0.f;
  if (state.run_oscillation > 1.f) state.run_oscillation = 1.f;

  float run_bounce_height = 200.f * state.run_oscillation;
  float run_cycle_time = 2.f * t_TAU * (fmodf(state.animation_seek_time, 8.f) / 8.f) + t_HALF_PI;
  float run_bounce_cycle = 0.5f + 0.5f * sinf(run_cycle_time);

  body_position.y += run_bounce_height * run_bounce_cycle;
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
    float player_speed = state.player_velocity.magnitude();

    HandleRunOscillation(state, body_position, player_speed);
  } else {
    // Player death
    float death_alpha = 2.f * time_since(state.death_time);
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

  // Head
  {
    auto& head = objects(meshes.player_head)[0];
    auto& pose = state.player_current_pose;
    auto& head_bone = pose.bones[0];

    head.position = body_position + body_rotation.toMatrix4f() * (head_bone.translation * 1100.f);
    head.rotation = body_rotation * head_bone.rotation;
    head.scale = tVec3f(1500.f);
    head.color = tVec3f(0, 0, 0.1f);
    head.material = tVec4f(1.f, 0, 0, 0);

    commit(head);
  }

  // Clothing
  {

    auto& hood = skinned_mesh(meshes.player_hood);
    auto& robes = skinned_mesh(meshes.player_robes);
    auto& shirt = skinned_mesh(meshes.player_shirt);
    auto& pants = skinned_mesh(meshes.player_pants);
    auto& boots = skinned_mesh(meshes.player_boots);

    hood.position = body_position;
    hood.rotation = body_rotation;
    hood.scale = body_scale;
    hood.color = tVec3f(0, 0.2f, 0.6f);
    hood.material = tVec4f(1.f, 0, 0, 0.5f);
    hood.shadow_cascade_ceiling = 2;

    robes.position = body_position;
    robes.rotation = body_rotation;
    robes.scale = body_scale;
    robes.color = tVec3f(0.3f, 0.4f, 0.6f);
    robes.material = tVec4f(1.f, 0, 0, 0.5f);
    robes.shadow_cascade_ceiling = 2;

    shirt.position = body_position;
    shirt.rotation = body_rotation;
    shirt.scale = body_scale;
    shirt.color = tVec3f(0.3f, 0.1f, 0.3f);
    shirt.material = tVec4f(1.f, 0, 0, 0.2f);
    shirt.shadow_cascade_ceiling = 2;

    pants.position = body_position;
    pants.rotation = body_rotation;
    pants.scale = body_scale;
    pants.color = tVec3f(0.4f, 0.2f, 0.1f);
    pants.material = tVec4f(1.f, 0, 0, 0.2f);
    pants.shadow_cascade_ceiling = 2;

    boots.position = body_position;
    boots.rotation = body_rotation;
    boots.scale = body_scale;
    boots.color = tVec3f(0.1f, 0.1f, 0.1f);
    boots.material = tVec4f(1.f, 0, 0, 0);
    boots.shadow_cascade_ceiling = 2;

    hood.current_pose = &state.player_current_pose;
    robes.current_pose = &state.player_current_pose;
    shirt.current_pose = &state.player_current_pose;
    pants.current_pose = &state.player_current_pose;
    boots.current_pose = &state.player_current_pose;

    commit(hood);
    commit(robes);
    commit(shirt);
    commit(pants);
    commit(boots);
  }

  if (state.show_game_stats) {
    ShowDebugPlayerSkeleton(tachyon, state, player_rotation, player_rotation_matrix);
  } else {
    reset_instances(meshes.debug_skeleton_bone);
  }
}

static void UpdateWand(Tachyon* tachyon, State& state, Quaternion& player_rotation, tMat4f& player_rotation_matrix) {
  auto& wand = objects(state.meshes.wand)[0];

  tVec3f offset = player_rotation_matrix * tVec3f(-1.f, 0, 0);

  wand.scale = tVec3f(800.f);
  wand.color = tVec3f(1.f, 0.6f, 0.2f);
  wand.material = tVec4f(1.f, 0, 0, 0.4f);

  if (state.player_hp > 0.f) {
    // Sync the wand to the player's right hand
    // @todo factor
    auto& pose = state.player_current_pose;
    auto& right_hand = pose.bones[5];
    tVec3f position = right_hand.translation;
    Quaternion rotation = right_hand.rotation;

    position += right_hand.rotation.toMatrix4f() * tVec3f(-0.05f, 0.32f, 0);

    wand.rotation =
      player_rotation * rotation *
      Quaternion::fromAxisAngle(tVec3f(0, 0, 1.f), 0.2f) *
      Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), 1.8f) *
      Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), t_PI);

    wand.position = state.player_position + player_rotation_matrix * (position * 1500.f);
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
    if (
      state.last_wand_swing_time != 0.f &&
      state.player_hp > 0.f
    ) {
      float time_since_last_swing = time_since(state.last_wand_swing_time);
      tVec3f player_right = tVec3f::cross(state.player_facing_direction, tVec3f(0, 1.f, 0));

      // Define the animation steps
      AnimationStep s1;
      s1.duration = 0.2f;
      s1.offset = tVec3f(0.f);
      s1.rotation = (
        // @hack
        // @todo predefine this
        state.player_current_pose.bones[5].rotation *
        Quaternion::fromAxisAngle(tVec3f(0, 0, 1.f), 0.2f) *
        Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), 1.8f) *
        Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), t_PI)
      );

      AnimationStep s2;
      s2.duration = 0.15f;
      s2.offset = tVec3f(0, 1500.f, 0) + player_right * 500.f;
      s2.rotation = (
        Quaternion::fromAxisAngle(tVec3f(0, 0, 1.f), 1.f) *
        Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), -1.f)
      );

      AnimationStep s3;
      s3.duration = 0.5f;
      s3.offset = state.player_facing_direction * 1000.f - player_right * 1000.f;
      s3.rotation = (
        Quaternion::fromAxisAngle(tVec3f(0, 0, 1.f), 2.f) *
        Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), 2.1f)
      );

      AnimationStep s4 = s1;
      s4.rotation = (
        // @hack
        // @todo predefine this
        state.player_current_pose.bones[5].rotation *
        Quaternion::fromAxisAngle(tVec3f(0, 0, 1.f), 0.2f) *
        Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), 1.8f) *
        Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), t_PI)
      );

      AnimationSequence swing_animation;
      swing_animation.steps = { s1, s2, s3, s4 };

      // Sample the animation
      TransformState sample = SimpleAnimation::Sample(swing_animation, time_since_last_swing);
      wand.position += sample.offset;
      wand.rotation = player_rotation * sample.rotation;

      if (
        time_since_last_swing > s1.duration &&
        time_since_last_swing < s1.duration + s2.duration
      ) {
        Combat::HandleWandStrikeWindow(tachyon, state);
      }

      // Allow the wand to revert to its original position after the swing completes
      // @temporary
      if (time_since_last_swing >= s1.duration + s2.duration + s3.duration) {
        state.last_wand_swing_time = 0.f;
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
      s1.rotation = (
        Quaternion::fromAxisAngle(tVec3f(0, 0, 1.f), 2.f) *
        Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), 2.1f)
      );

      AnimationStep s2;
      s2.duration = 0.5f;
      s2.offset = tVec3f(0, 1500.f, 0) + player_right * 500.f;
      s2.rotation = (
        Quaternion::fromAxisAngle(tVec3f(0, 0, 1.f), 1.f) *
        Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), -1.f)
      );

      AnimationStep s3;
      s3.offset = tVec3f(0.f);
      s3.rotation = Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), 0.f);

      AnimationSequence bounce_animation;
      bounce_animation.steps = { s1, s2, s3 };

      // Sample the animation
      TransformState sample = SimpleAnimation::Sample(bounce_animation, time_since_bounce);
      wand.position += sample.offset;
      wand.rotation = player_rotation * sample.rotation;

      if (time_since_bounce > s1.duration + s2.duration) {
        // Animation complete
        state.last_wand_bounce_time = 0.f;
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

      float death_alpha = 2.f * time_since(state.death_time);
      if (death_alpha > 1.f) death_alpha = 1.f;

      wand.position += state.player_velocity * (1.f - death_alpha) * state.dt;
    }
  }

  commit(wand);
}

static void UpdateLantern(Tachyon* tachyon, State& state, const tMat4f& player_rotation_matrix) {
  // Lantern object
  {
    // @todo
  }

  // Point light source
  {
    auto& light = *get_point_light(state.player_light_id);

    tVec3f offset = player_rotation_matrix * tVec3f(710.f, 50.f, 0);

    light.position = state.player_position + offset;
    light.position.y -= 300.f;
    light.radius = 2500.f;
    light.color = tVec3f(0.5f, 0.3f, 0.6f);
    light.color = get_point_light(state.astrolabe_light_id)->color;
    light.power = 0.5f;
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

  // Update facing direction
  {
    tVec3f desired_facing_direction = state.player_facing_direction;
    float turning_speed = 5.f;

    if (state.has_target) {
      // When we're focused on a target, face it and turn much more quickly
      auto& target = *EntityManager::FindEntity(state, state.target_entity);

      desired_facing_direction = (target.visible_position - state.player_position).xz().unit();
    }
    else if (state.player_velocity.magnitude() > 0.01f) {
      // Without a target, use our velocity vector to influence facing direction
      desired_facing_direction = state.player_velocity.unit();
    }

    // When astro turning, don't change our facing direction at all,
    // since targeted entities may jitter and jump about rapidly,
    // and we don't want the facing direction being thrown off
    if (abs(state.astro_turn_speed) > 0.05f) {
      turning_speed = 0.f;
    }

    state.player_facing_direction = tVec3f::lerp(state.player_facing_direction, desired_facing_direction, turning_speed * state.dt).unit();
  }

  Quaternion player_rotation = Quaternion::FromDirection(state.player_facing_direction, tVec3f(0, 1.f, 0));
  tMat4f player_rotation_matrix = player_rotation.toMatrix4f();

  UpdatePlayerModel(tachyon, state, player_rotation, player_rotation_matrix);
  UpdateWand(tachyon, state, player_rotation, player_rotation_matrix);
  UpdateLantern(tachyon, state, player_rotation_matrix);
}

void PlayerCharacter::AutoHop(Tachyon* tachyon, State& state) {
  if (time_since(state.last_auto_hop_time) > AUTO_HOP_DURATION) {
    state.last_auto_hop_time = get_scene_time();
  }
}

bool PlayerCharacter::CanTakeDamage(Tachyon* tachyon, const State& state) {
  return (
    time_since(state.last_damage_time) > 1.5f &&
    time_since(state.last_strong_attack_time) > 1.f
  );
}

void PlayerCharacter::TakeDamage(Tachyon* tachyon, State& state, const float damage) {
  state.player_hp -= damage;
  state.last_damage_time = get_scene_time();

  // Cancel any dodge motions
  state.last_dodge_time = 0.f;

  if (state.player_hp <= 0.f) {
    // @temporary
    UISystem::ShowBlockingDialogue(tachyon, state, "YOU DIED");

    state.death_time = get_scene_time();
    state.last_wand_swing_time = 0.f;
    state.last_wand_bounce_time = 0.f;
  }
}