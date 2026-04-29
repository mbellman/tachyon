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
  float blend_alpha = fmodf(seek_time, 1.f);
  float t = seek_time;

  // @temporary @todo Do this when loading the animations for the first time
  ReserveAnimationPoseData(animation);

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
    float smoothing = 1.f;
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
    Quaternion blended_rotation = Quaternion::nlerp(current_bone.rotation, next_bone.rotation, blend_alpha);

    animation.evaluated_pose.bones[i].rotation = blended_rotation;

    // @todo blend translations (+ scale??)
  }
}

void Animation::AccumulateTime(tSkinnedMeshAnimation& mesh_animation, const float animation_speed, const float blend_rate, const float dt) {
  mesh_animation.seek_time += animation_speed * dt;

  // Limit and wrap the animation time to a common multiple of the
  // current/next animation times so that we don't eventually encounter
  // accumulation precision errors
  {
    float max_seek_time = GetMaxSeekTime(*mesh_animation.current_animation) * GetMaxSeekTime(*mesh_animation.next_animation);

    if (mesh_animation.seek_time > max_seek_time) {
      mesh_animation.seek_time -= max_seek_time;
    } else if (mesh_animation.seek_time < 0.f) {
      mesh_animation.seek_time = max_seek_time;
    }
  }

  // Increment the blend factor to the next animation, in case
  // we're transitioning between animations.
  {
    mesh_animation.next_animation_blend_alpha += blend_rate * dt;

    if (mesh_animation.next_animation_blend_alpha > 1.f) {
      mesh_animation.next_animation_blend_alpha = 1.f;
    }
  }

  // Update the current animation when a full blend from current -> next is complete
  if (
    mesh_animation.current_animation != mesh_animation.next_animation &&
    mesh_animation.next_animation_blend_alpha == 1.f
  ) {
    mesh_animation.current_animation = mesh_animation.next_animation;
  }
}

void Animation::UpdatePose(tSkinnedMeshAnimation& mesh_animation) {
  tSkeletonAnimation& current_animation = *mesh_animation.current_animation;
  tSkeletonAnimation& next_animation = *mesh_animation.next_animation;

  // Evaluate the current and next animations simultaneously so they can be blended
  // @optimize this only has to be done when transitioning between animations
  EvaluateAnimation(current_animation, mesh_animation.seek_time);
  EvaluateAnimation(next_animation, mesh_animation.seek_time);

  if (mesh_animation.upper_body_animation != nullptr) {
    EvaluateAnimation(*mesh_animation.upper_body_animation, mesh_animation.upper_body_animation_time);
  }

  // Update the active pose based on the blended result of the current/next animations
  {
    auto& active_pose = mesh_animation.active_pose;
    // @todo make blend type configurable
    float blend_alpha = mesh_animation.next_animation_blend_alpha;
    // float blend_alpha = Tachyon_EaseInOutf(mesh_animation.next_animation_blend_alpha);

    // Compute the active pose rotation by blending between the current/next animations
    // @optimize if the current and next animations are identical, this is unnecessary
    for (size_t i = 0; i < current_animation.evaluated_pose.bones.size(); i++) {
      auto& current_bone = current_animation.evaluated_pose.bones[i];
      auto& next_bone = next_animation.evaluated_pose.bones[i];
      auto& active_pose_bone = active_pose.bones[i];
      Quaternion blended_rotation = Quaternion::nlerp(current_bone.rotation, next_bone.rotation, blend_alpha);

      // Reset current pose bone translation back to bone space
      active_pose_bone.translation = current_bone.translation;

      // Set blended rotation
      active_pose_bone.rotation = blended_rotation;

      // @todo allow the torso bone name to be specified
      if (active_pose_bone.name == "Torso") {
        active_pose_bone.rotation *= Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), mesh_animation.torso_turn_angle);
      }
    }

    // Handle separate upper body animation
    if (mesh_animation.upper_body_animation != nullptr) {
      auto& animation = *mesh_animation.upper_body_animation;
      float seek_time = mesh_animation.upper_body_animation_time;
      float progress = seek_time / float(animation.frames.size());
      if (progress > 1.f) progress = 1.f;
      // @todo make blend alpha configurable. Right now this assumes
      // we play the upper body once, ramping it up and then down again
      // as it completes. We may have cases where we want an upper body
      // animation to play cyclically.
      float blend_alpha = SmoothStep(0.f, 0.05f, progress) * (1.f - SmoothStep(0.7f, 1.f, progress));

      for (size_t i = 0; i < active_pose.bones.size(); i++) {
        auto& active_pose_bone = active_pose.bones[i];
        auto& animation = *mesh_animation.upper_body_animation;
        auto& bone_name = active_pose_bone.name;

        // Skip lower-body bones
        if (bone_name.starts_with("Pelvis")) continue;
        if (bone_name.starts_with("Thigh")) continue;
        if (bone_name.starts_with("Shin")) continue;
        if (bone_name.starts_with("Foot")) continue;

        auto& upper_bone = animation.evaluated_pose.bones[active_pose_bone.index];

        active_pose_bone.rotation = Quaternion::nlerp(active_pose_bone.rotation, upper_bone.rotation, blend_alpha);
      }
    }

    // Handle right arm animation
    if (mesh_animation.arm_animation != nullptr) {
      // @todo ?
    }
  }
}

void Animation::UpdateBoneMatrices(tSkinnedMeshAnimation& mesh_animation) {
  auto& rest_pose = mesh_animation.rest_pose;
  auto& active_pose = mesh_animation.active_pose;

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

    // @todo allow the head bone name to be specified
    if (bone.name == "Head") {
      bone.rotation *= Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), mesh_animation.head_turn_angle);
    }

    tMat4f& inverse_bind_matrix = rest_pose.bone_matrices[bone.index];
    tMat4f pose_matrix = tMat4f::transformation(bone.translation, tVec3f(1.f), bone.rotation);
    tMat4f bone_matrix = pose_matrix * inverse_bind_matrix;

    active_pose.bone_matrices.push_back(bone_matrix.transpose());
  }
}

void Animation::SetNextAnimation(tSkinnedMeshAnimation& mesh_animation, tSkeletonAnimation* skeleton_animation) {
  if (mesh_animation.next_animation == skeleton_animation) {
    return;
  }

  mesh_animation.next_animation = skeleton_animation;
  mesh_animation.next_animation_blend_alpha = 0.f;
}

void Animation::StartNextAnimation(tSkinnedMeshAnimation& mesh_animation, tSkeletonAnimation* skeleton_animation) {
  if (mesh_animation.next_animation == skeleton_animation) {
    return;
  }

  mesh_animation.current_animation = skeleton_animation;
  mesh_animation.next_animation = skeleton_animation;
  mesh_animation.next_animation_blend_alpha = 0.f;
  mesh_animation.seek_time = 0.f;
}

void Animation::AwaitNextAnimation(tSkinnedMeshAnimation& mesh_animation, tSkeletonAnimation* skeleton_animation) {
  if (mesh_animation.next_animation != nullptr && mesh_animation.next_animation_blend_alpha < 1.f) {
    return;
  }

  Animation::SetNextAnimation(mesh_animation, skeleton_animation);
}