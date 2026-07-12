#pragma once

#include "astro/game_state.h"

namespace astro {
  namespace SoundDriver {
    void PlayWalkSound(State& state, const float volume);
    void PlayLadderSound(State& state, const float volume);
  }
}