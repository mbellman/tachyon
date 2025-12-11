#include "astro/simple_animation.h"

using namespace astro;

TransformState SimpleAnimation::Sample(const AnimationSequence& sequence, float time) {
  TransformState transform;
  float running_time = 0.f;

  for (size_t i = 0; i < sequence.steps.size() - 1; i++) {
    auto& current_step = sequence.steps[i];
    auto& next_step = sequence.steps[i + 1];

    if (time < running_time + current_step.duration) {
      // Active step
      float alpha = Tachyon_InverseLerp(running_time, running_time + current_step.duration, time);

      transform.offset = tVec3f::lerp(current_step.offset, next_step.offset, alpha);
      transform.rotation = Quaternion::slerp(current_step.rotation, next_step.rotation, alpha);

      break;
    }

    running_time += current_step.duration;
  }

  return transform;
}