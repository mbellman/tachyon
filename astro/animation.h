#pragma once

#include "astro/game_state.h"

namespace astro {
  enum AnimationBlendType {
    BLEND_LINEAR,
    BLEND_EASE_IN_OUT
  };

  // @todo move to engine (?)
  namespace Animation {
    void AccumulateTime(tSkinnedMeshAnimation& mesh_animation, const float animation_speed, const float blend_rate, const float dt);
    void UpdatePose(tSkinnedMeshAnimation& mesh_animation, const AnimationBlendType blend_type);
    void UpdateBoneMatrices(tSkinnedMeshAnimation& mesh_animation);
    void SetNextAnimation(tSkinnedMeshAnimation& mesh_animation, tSkeletonAnimation* skeleton_animation);
    void StartNextAnimation(tSkinnedMeshAnimation& mesh_animation, tSkeletonAnimation* skeleton_animation);
    void AwaitNextAnimation(tSkinnedMeshAnimation& mesh_animation, tSkeletonAnimation* skeleton_animation);
  }
}