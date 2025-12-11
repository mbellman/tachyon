#pragma once

#include "engine/tachyon.h"

namespace astro {
  struct TransformState {
    tVec3f offset;
    Quaternion rotation;
  };

  struct AnimationStep : TransformState{
    float duration;
  };

  struct AnimationSequence {
    std::vector<AnimationStep> steps;
  };

  namespace SimpleAnimation {
    TransformState Sample(const AnimationSequence& sequence, float time);
  }
}