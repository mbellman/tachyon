#pragma once

#include "astro/game_state.h"

namespace astro {
  // @todo move to engine (?)
  namespace Animation {
    void AccumulateTime(tSkinnedMeshAnimation& mesh_animation, const float speed, const float dt);
    void UpdatePose(tSkinnedMeshAnimation& mesh_animation);
    void UpdateBoneMatrices(tSkinnedMeshAnimation& mesh_animation);
    void SetNextAnimation(tSkinnedMeshAnimation& mesh_animation, tSkeletonAnimation* skeleton_animation);
    void AwaitNextAnimation(tSkinnedMeshAnimation& mesh_animation, tSkeletonAnimation* skeleton_animation);
  }
}