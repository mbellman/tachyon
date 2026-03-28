#include "astro/animation.h"

using namespace astro;

static void ReserveAnimationPoseData(tSkeletonAnimation& skeleton_animation) {
  if (skeleton_animation.evaluated_pose.bones.size() == 0) {
    for (auto& bone : skeleton_animation.frames[0].bones) {
      skeleton_animation.evaluated_pose.bones.push_back(bone);
    }
  }
}

static void EvaluateAnimation(tSkeletonAnimation& animation, const float seek_time) {
  float blend_alpha = fmodf(seek_time, 1.f);

  if (animation.frames.size() == 2) {
    // Special treatment for 2-frame animations: alternate between
    // both frames using an ease-in-out transition
    blend_alpha = Tachyon_EaseInOutf(blend_alpha);
  }

  int32 current_frame_index = int(seek_time) % animation.frames.size();
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

static float GetMaxSeekTime(tSkeletonAnimation& animation) {
  return (float)animation.frames.size();
}

// @todo blend time as a parameter
void Animation::AccumulateTime(tSkinnedMeshAnimation& mesh_animation, const float accumulation, const float dt) {
  mesh_animation.seek_time += accumulation * dt;

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

  // Track time between changing animations so they can be blended
  {
    mesh_animation.time_since_last_animation_change += 2.f * dt;

    if (mesh_animation.time_since_last_animation_change > 1.f) {
      mesh_animation.time_since_last_animation_change = 1.f;
    }
  }

  // Update the current animation when a full blend from current -> next is complete
  if (
    mesh_animation.current_animation != mesh_animation.next_animation &&
    mesh_animation.time_since_last_animation_change == 1.f
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

  // Update the active pose based on the blended result of the current/next animations
  {
    auto& active_pose = mesh_animation.active_pose;
    float blend_alpha = mesh_animation.time_since_last_animation_change;

    for (size_t i = 0; i < current_animation.evaluated_pose.bones.size(); i++) {
      auto& previous_bone = current_animation.evaluated_pose.bones[i];
      auto& next_bone = next_animation.evaluated_pose.bones[i];
      auto& active_pose_bone = active_pose.bones[i];
      Quaternion blended_rotation = Quaternion::nlerp(previous_bone.rotation, next_bone.rotation, blend_alpha);

      // Reset current pose bone translation back to bone space
      active_pose_bone.translation = previous_bone.translation;

      // Set blended rotation
      active_pose_bone.rotation = blended_rotation;
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

    tMat4f inverse_bind_matrix = rest_pose.bone_matrices[bone.index];
    tMat4f pose_matrix = tMat4f::transformation(bone.translation, tVec3f(1.f), bone.rotation);
    tMat4f bone_matrix = pose_matrix * inverse_bind_matrix;

    active_pose.bone_matrices.push_back(bone_matrix.transpose());
  }
}

void Animation::SetNextAnimation(tSkinnedMeshAnimation& mesh_animation, tSkeletonAnimation* skeleton_animation) {
  if (mesh_animation.next_animation == skeleton_animation) {
    return;
  }

  ReserveAnimationPoseData(*skeleton_animation);

  mesh_animation.next_animation = skeleton_animation;
  mesh_animation.time_since_last_animation_change = 0.f;
}

void Animation::AwaitNextAnimation(tSkinnedMeshAnimation& mesh_animation, tSkeletonAnimation* skeleton_animation) {
  if (mesh_animation.next_animation != nullptr && mesh_animation.time_since_last_animation_change < 1.f) {
    return;
  }

  Animation::SetNextAnimation(mesh_animation, skeleton_animation);
}