#pragma once

#include "astro/game_state.h"

namespace astro {
  enum AnimationBlendType {
    BLEND_LINEAR,
    BLEND_EASE_IN_OUT
  };

  // @todo move to engine (?)
  namespace Animation {
    void AccumulateTime(tAnimationRig& rig, const float blend_rate, const float dt);
    void UpdatePose(tAnimationRig& rig, const AnimationBlendType blend_type);
    void UpdateBoneMatrices(tAnimationRig& rig);
    void SetNextAnimation(tAnimationRig& rig, tSkeletonAnimation* skeleton_animation);
    void StartNextAnimation(tAnimationRig& rig, tSkeletonAnimation* skeleton_animation);
    void AwaitNextAnimation(tAnimationRig& rig, tSkeletonAnimation* skeleton_animation);
    float GetMaxTime(tSkeletonAnimation* animation);
    tVec3f GetRootMotion(tAnimationRig& rig);
  }
}