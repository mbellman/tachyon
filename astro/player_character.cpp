#include "astro/player_character.h"
#include "astro/combat.h"
#include "astro/entity_manager.h"
#include "astro/sfx.h"
#include "astro/simple_animation.h"
#include "astro/ui_system.h"

using namespace astro;

const static float AUTO_HOP_DURATION = 0.3f;

static void UpdatePlayerSkeleton(Tachyon* tachyon, State& state) {
  profile("UpdatePlayerSkeleton()");

  // @temporary
  // @todo blend between animations
  auto& animation = state.player_velocity.magnitude() > 50.f
    ? state.animations.player_walk
    : state.animations.player_idle;

  float seek_time = tachyon->running_time * animation.speed;
  float blend_alpha = fmodf(seek_time, 1.f);

  if (animation.frames.size() == 2) {
    // Special treatment for 2-frame animations: alternate between
    // both frames using an ease-in-out transition
    blend_alpha = Tachyon_EaseInOutf(blend_alpha);
  }

  int32 start_index = int(seek_time) % animation.frames.size();
  int32 end_index = (start_index + 1) % animation.frames.size();
  auto& start_skeleton = animation.frames[start_index];
  auto& end_skeleton = animation.frames[end_index];

  for (int32 i = 0; i < start_skeleton.bones.size(); i++) {
    auto& start_bone = start_skeleton.bones[i];
    auto& end_bone = end_skeleton.bones[i];
    Quaternion blended_rotation = Quaternion::nlerp(start_bone.rotation, end_bone.rotation, blend_alpha);

    state.player_skeleton.bones[i].rotation = blended_rotation;

    // @todo blend translations (+ scale??)
  }

  // @todo precompute object-space bone rotations/offsets
}

// @todo debug mode only
static void ShowDebugPlayerSkeleton(Tachyon* tachyon, State& state, Quaternion& player_rotation, tMat4f& player_rotation_matrix) {
  profile("ShowDebugPlayerSkeleton()");

  auto& camera = tachyon->scene.camera;
  auto& meshes = state.meshes;

  tVec3f camera_to_player = state.player_position - camera.position;
  tVec3f base_position = camera.position + camera_to_player.unit() * 650.f;

  reset_instances(meshes.debug_skeleton_bone);

  auto& skeleton = state.player_skeleton;

  for (auto& bone : skeleton.bones) {
    tVec3f bone_translation = bone.translation;
    Quaternion bone_rotation = bone.rotation;
    int32 next_parent_index = bone.parent_bone_index;

    // Offset the bone so that it can be represented as a stick
    // in between its translation and the next bone, and not just
    // a point at its initial translation coordinate
    //
    // @bug this is still slightly wrong sometimes
    bone_translation += bone.rotation.toMatrix4f() * (bone.translation * 0.5f);

    while (next_parent_index != -1) {
      auto& parent_bone = skeleton.bones[next_parent_index];

      bone_translation = parent_bone.translation + parent_bone.rotation.toMatrix4f() * bone_translation;
      bone_rotation = parent_bone.rotation * bone_rotation;
      next_parent_index = parent_bone.parent_bone_index;
    }

    auto& debug_bone = use_instance(meshes.debug_skeleton_bone);
    float bone_length = bone.translation.magnitude() * 25.f;

    // @todo cleanup
    if (bone.parent_bone_index != -1) {
      auto& parent = skeleton.bones[bone.parent_bone_index];
      float parent_bone_length = parent.translation.magnitude() * 25.f;

      if (parent_bone_length > bone_length) {
        bone_length = parent_bone_length;
      }
    }

    // @todo cleanup
    if (bone.child_bone_indexes.size() > 0) {
      auto& child = skeleton.bones[bone.child_bone_indexes[0]];
      float child_bone_length = child.translation.magnitude() * 25.f;

      if (child_bone_length > bone_length) {
        bone_length = child_bone_length;
      }
    }

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

static void UpdatePlayerModel(Tachyon* tachyon, State& state, Quaternion& rotation, tMat4f& rotation_matrix) {
  auto& meshes = state.meshes;

  auto& player = objects(meshes.player)[0];

  player.scale = tVec3f(1500.f);
  player.color = tVec3f(0, 0, 0.1f);
  player.material = tVec4f(1.f, 0, 0, 0);

  // Auto-hop actions
  {
    if (
      state.last_auto_hop_time != 0.f &&
      time_since(state.last_auto_hop_time) < AUTO_HOP_DURATION
    ) {
      HandleAutoHop(state);
    }
  }

  // Taking damage
  // @temporary
  if (
    state.last_damage_time != 0.f &&
    time_since(state.last_damage_time) < 1.5f
  ) {
    player.color = tVec3f(1.f, 0, 0);
  }

  if (state.player_hp <= 0.f) {
    // Player death
    float death_alpha = 2.f * time_since(state.death_time);
    if (death_alpha > 1.f) death_alpha = 1.f;

    // @temporary
    tVec3f death_rotation_axis = tVec3f(1.f, 0, 0);
    Quaternion death_rotation = rotation * Quaternion::fromAxisAngle(death_rotation_axis, -t_HALF_PI);

    player.rotation = Quaternion::slerp(player.rotation, death_rotation, 4.f * state.dt);
    player.position.y = Tachyon_Lerpf(player.position.y, -1100.f, 4.f * state.dt);
    player.position += state.player_velocity * (1.f - death_alpha) * state.dt;
  } else {
    player.rotation = rotation;
    player.position = state.player_position;
  }

  commit(player);

  // Clothing
  {
    auto& clothing = objects(meshes.player_clothing)[0];

    clothing.position = player.position;
    clothing.rotation = player.rotation;
    clothing.scale = player.scale;
    clothing.color = tVec3f(0, 0.2f, 0.6f);
    clothing.material = tVec4f(1.f, 0, 0, 0.2f);

    commit(clothing);
  }

  // Boots
  {
    auto& boots = objects(meshes.player_boots)[0];

    boots.position = player.position;
    boots.rotation = player.rotation;
    boots.scale = player.scale;
    boots.color = 0x2110;
    boots.material = tVec4f(1.f, 0, 0, 0.4f);

    commit(boots);
  }

  UpdatePlayerSkeleton(tachyon, state);

  if (state.show_game_stats) {
    ShowDebugPlayerSkeleton(tachyon, state, rotation, rotation_matrix);
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
    wand.position = state.player_position + offset * 900.f;
    wand.rotation = player_rotation;
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
      s1.rotation = Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), 0.f);

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

  // Astro light
  {
    auto& light = *get_point_light(state.player_light_id);

    tVec3f offset = player_rotation_matrix * tVec3f(1.f, 0, 0);

    light.position = state.player_position + offset * 1000.f;
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