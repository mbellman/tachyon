#include "astro/animation.h"

using namespace astro;

// @todo move to engine
static float SmoothStep(const float e1, const float e2, float x) {
  x = (x - e1) / (e2 - e1);
  if (x < 0.f) x = 0.f;
  if (x > 1.f) x = 1.f;

  return x * x * (3.f - 2.f * x);
}

static void ReserveAnimationPoseData(tSkeletonAnimation& skeleton_animation) {
  if (skeleton_animation.evaluated_pose.bones.size() == 0) {
    for (auto& bone : skeleton_animation.frames[0].bones) {
      skeleton_animation.evaluated_pose.bones.push_back(bone);
    }
  }
}

static float GetMaxSeekTime(tSkeletonAnimation& animation) {
  return (float) animation.frames.size();
}

static float Curve(const float t, const float k) {
  return t + k * (3.f * t * t - 2.f * t * t * t - t);
}

static void EvaluateAnimation(tSkeletonAnimation& animation, const float seek_time) {
  int total_frames = (int) animation.frames.size();
  float max_time = GetMaxSeekTime(animation);
  float blend_alpha = fmodf(seek_time, 1.f);
  float t = seek_time;

  // @temporary @todo Do this when loading the animations for the first time
  ReserveAnimationPoseData(animation);

  if (total_frames== 2) {
    // Special treatment for 2-frame animations: alternate between
    // both frames using an ease-in-out transition
    blend_alpha = Tachyon_EaseInOutf(blend_alpha);
  } else {
    // Determine seek time / max time ratio
    t = fmodf(t, max_time);
    t = t / max_time;

    if (!animation.looping && seek_time == max_time) {
      // If the animation is non-looping, represent t as 1
      // here so it can be scaled back up to max_time
      t = 1.f;
    }

    // Allow for smoother time curves as opposed to tracking progress linearly
    float smoothing = 0.2f;

    if (t < 0.5f) t = 0.5f * Curve(t * 2.f, smoothing);
    else          t = 0.5f + 0.5f * Curve((t - 0.5f) * 2.f, smoothing);

    // Scale back up to the max time to resolve our adjusted seek time
    t *= max_time;

    // Determine blend between the current/next keyframes
    blend_alpha = fmodf(t, 1.f);
  }

  int32 current_frame_index;
  int32 next_frame_index;

  if (animation.looping) {
    current_frame_index = int(t) % animation.frames.size();
    next_frame_index = (current_frame_index + 1) % animation.frames.size();
  } else {
    // Frame cycling for non-looping animations, which we want to end
    // on the final frame exactly at t = max_time
    current_frame_index = (int32) floorf(t);
    next_frame_index = (int32) ceilf(t);

    if (current_frame_index == total_frames) current_frame_index -= 1;
    if (next_frame_index == total_frames) next_frame_index -= 1;
  }

  auto& current_frame = animation.frames[current_frame_index];
  auto& next_frame = animation.frames[next_frame_index];

  for (size_t i = 0; i < current_frame.bones.size(); i++) {
    auto& current_bone = current_frame.bones[i];
    auto& next_bone = next_frame.bones[i];
    Quaternion blended_rotation = Quaternion::nlerp(current_bone.rotation, next_bone.rotation, blend_alpha);

    animation.evaluated_pose.bones[i].rotation = blended_rotation;

    // @todo blend translations (+ scale??)
  }
}

// @todo refactor with EvaluateAnimation()
static tVec3f GetSkeletonRootMotion(tSkeletonAnimation& animation, const float seek_time) {
  float blend_alpha = fmodf(seek_time, 1.f);
  float t = seek_time;

  if (animation.frames.size() == 2) {
    // Special treatment for 2-frame animations: alternate between
    // both frames using an ease-in-out transition
    blend_alpha = Tachyon_EaseInOutf(blend_alpha);
  } else {
    // Determine seek time / max time ratio
    float max_time = GetMaxSeekTime(animation);
    t = fmodf(seek_time, max_time);
    t = t / max_time;

    // Allow for smoother time curves as opposed to tracking progress linearly
    float smoothing = 0.2f;

    if (t < 0.5f) t = 0.5f * Curve(t * 2.f, smoothing);
    else          t = 0.5f + 0.5f * Curve((t - 0.5f) * 2.f, smoothing);

    // Scale back up to the max time to resolve our adjusted seek time
    t *= max_time;

    // Determine blend between the current/next keyframes
    blend_alpha = fmodf(t, 1.f);
  }

  int32 current_frame_index = int(t) % animation.frames.size();
  int32 next_frame_index = (current_frame_index + 1) % animation.frames.size();

  auto& current_frame = animation.frames[current_frame_index];
  auto& next_frame = animation.frames[next_frame_index];

  for (size_t i = 0; i < current_frame.bones.size(); i++) {
    auto& current_bone = current_frame.bones[i];
    auto& next_bone = next_frame.bones[i];

    if (current_bone.name == "Root") {
      return tVec3f::lerp(current_bone.translation, next_bone.translation, blend_alpha);
    }
  }

  return tVec3f(0.f);
}

void Animation::AccumulateTime(tAnimationRig& rig, const float blend_rate, const float dt) {
  // Update and wrap current animation seek time
  // @todo refactor with below
  {
    auto& animation = *rig.current_animation;
    float max_seek_time = GetMaxSeekTime(animation);

    rig.current_animation_time += rig.current_animation_speed * dt;

    if (rig.current_animation_time > max_seek_time) {
      if (rig.current_animation->looping) {
        rig.current_animation_time -= max_seek_time;
      } else {
        rig.current_animation_time = max_seek_time;
      }
    } else if (rig.current_animation_time < 0.f) {
      rig.current_animation_time += max_seek_time;
    }
  }

  // Update and wrap next animation seek time
  // @todo refactor with above
  if (rig.next_animation != rig.current_animation) {
    auto& animation = *rig.next_animation;
    float max_seek_time = GetMaxSeekTime(animation);

    rig.next_animation_time += rig.next_animation_speed * dt;

    if (rig.next_animation_time > max_seek_time) {
      if (rig.next_animation->looping) {
        rig.next_animation_time -= max_seek_time;
      } else {
        rig.next_animation_time = max_seek_time;
      }
    } else if (rig.next_animation_time < 0.f) {
      rig.next_animation_time += max_seek_time;
    }
  } else {
    rig.next_animation_time = rig.current_animation_time;
  }

  // Increment the blend factor to the next animation, in case
  // we're transitioning between animations.
  {
    rig.next_animation_blend_alpha += blend_rate * dt;

    if (rig.next_animation_blend_alpha > 1.f) {
      rig.next_animation_blend_alpha = 1.f;
    }
  }

  // Update the current animation when a full blend from current -> next is complete
  if (
    rig.current_animation != rig.next_animation &&
    rig.next_animation_blend_alpha == 1.f
  ) {
    rig.current_animation = rig.next_animation;

    // Carry over the animation time to avoid skipping behavior
    rig.current_animation_time = rig.next_animation_time;
  }
}

void Animation::UpdatePose(tAnimationRig& rig, const AnimationBlendType blend_type) {
  tSkeletonAnimation& current_animation = *rig.current_animation;
  tSkeletonAnimation& next_animation = *rig.next_animation;

  // Evaluate the current and next animations simultaneously so they can be blended
  // @optimize this only has to be done when transitioning between animations
  EvaluateAnimation(current_animation, rig.current_animation_time);
  EvaluateAnimation(next_animation, rig.next_animation_time);

  if (rig.upper_body_animation != nullptr) {
    EvaluateAnimation(*rig.upper_body_animation, rig.upper_body_animation_time);
  }

  // Update the active pose based on the blended result of the current/next animations
  {
    auto& active_pose = rig.active_pose;
    float blend_alpha;

    if (blend_type == BLEND_EASE_IN_OUT) {
      blend_alpha = Tachyon_EaseInOutf(rig.next_animation_blend_alpha);
    } else {
      blend_alpha = rig.next_animation_blend_alpha;
    }

    // Compute the active pose rotation by blending between the current/next animations
    // @optimize if the current and next animations are identical, this is unnecessary
    for (size_t i = 0; i < current_animation.evaluated_pose.bones.size(); i++) {
      auto& current_bone = current_animation.evaluated_pose.bones[i];
      auto& next_bone = next_animation.evaluated_pose.bones[i];
      auto& active_bone = active_pose.bones[i];
      Quaternion blended_rotation = Quaternion::nlerp(current_bone.rotation, next_bone.rotation, blend_alpha);

      // Reset active pose bone translation back to bone space
      active_bone.translation = current_bone.translation;

      // Set blended rotation
      active_bone.rotation = blended_rotation;

      // @todo allow the torso bone name to be specified
      if (active_bone.name == "Torso") {
        active_bone.rotation *= Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), rig.torso_tilt_angle);
        active_bone.rotation *= Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), rig.torso_turn_angle);
        active_bone.translation *= 1.f - rig.torso_compression;
      }

      // @todo allow the head bone name to be specified
      if (active_bone.name == "Head") {
        active_bone.rotation *= Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), rig.head_turn_angle);
      }
    }

    // Handle separate upper body animation
    if (rig.upper_body_animation != nullptr) {
      auto& animation = *rig.upper_body_animation;
      float seek_time = rig.upper_body_animation_time;
      float progress = seek_time / float(animation.frames.size());
      if (progress > 1.f) progress = 1.f;
      // @todo make blend alpha configurable. Right now this assumes
      // we play the upper body once, ramping it up and then down again
      // as it completes. We may have cases where we want an upper body
      // animation to play cyclically.
      float blend_alpha = SmoothStep(0.f, 0.05f, progress) * (1.f - SmoothStep(0.7f, 1.f, progress));

      for (size_t i = 0; i < active_pose.bones.size(); i++) {
        auto& active_bone = active_pose.bones[i];
        auto& animation = *rig.upper_body_animation;
        auto& bone_name = active_bone.name;

        // Skip lower-body bones
        if (bone_name.starts_with("Pelvis")) continue;
        if (bone_name.starts_with("Thigh")) continue;
        if (bone_name.starts_with("Shin")) continue;
        if (bone_name.starts_with("Foot")) continue;

        auto& upper_bone = animation.evaluated_pose.bones[active_bone.index];

        active_bone.rotation = Quaternion::nlerp(active_bone.rotation, upper_bone.rotation, blend_alpha);
      }
    }
  }
}

void Animation::UpdateBoneMatrices(tAnimationRig& rig) {
  auto& rest_pose = rig.rest_pose;
  auto& active_pose = rig.active_pose;

  active_pose.bone_matrices.clear();

  for (auto& bone : active_pose.bones) {
    int32 next_parent_index = bone.parent_bone_index;

    // @todo refactor to use TransformBonesIntoMeshSpace()
    while (next_parent_index != -1) {
      auto& parent_bone = active_pose.bones[next_parent_index];

      bone.translation = parent_bone.translation + parent_bone.rotation.toMatrix4f() * bone.translation;
      bone.rotation = parent_bone.rotation * bone.rotation;

      next_parent_index = parent_bone.parent_bone_index;
    }

    tMat4f& inverse_bind_matrix = rest_pose.bone_matrices[bone.index];
    tMat4f pose_matrix = tMat4f::transformation(bone.translation, tVec3f(1.f), bone.rotation);
    tMat4f bone_matrix = pose_matrix * inverse_bind_matrix;

    active_pose.bone_matrices.push_back(bone_matrix.transpose());
  }
}

void Animation::SetNextAnimation(tAnimationRig& rig, tSkeletonAnimation* skeleton_animation) {
  if (rig.next_animation == skeleton_animation) {
    return;
  }

  rig.next_animation = skeleton_animation;
  rig.next_animation_time = 0.f;
  rig.next_animation_blend_alpha = 0.f;
}

void Animation::StartNextAnimation(tAnimationRig& rig, tSkeletonAnimation* skeleton_animation) {
  if (rig.next_animation == skeleton_animation) {
    return;
  }

  rig.next_animation = skeleton_animation;
  rig.next_animation_time = 0.f;
  rig.next_animation_blend_alpha = 0.f;
}

void Animation::AwaitNextAnimation(tAnimationRig& rig, tSkeletonAnimation* skeleton_animation) {
  if (rig.next_animation != nullptr && rig.next_animation_blend_alpha < 1.f) {
    return;
  }

  Animation::SetNextAnimation(rig, skeleton_animation);
}

tVec3f Animation::GetRootMotion(tAnimationRig& rig) {
  tVec3f initial_root = rig.current_animation->frames[0].bones[17].translation;
  tVec3f current_root = GetSkeletonRootMotion(*rig.current_animation, rig.current_animation_time);
  tVec3f next_root = GetSkeletonRootMotion(*rig.next_animation, rig.next_animation_time);
  tVec3f blended_root = tVec3f::lerp(current_root, next_root, rig.next_animation_blend_alpha);

  return blended_root - initial_root;
}