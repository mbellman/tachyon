#pragma once

namespace astro {
  enum Music {
    DIVINATION_WOODREALM
  };

  namespace BGM {
    void PlayMusic(Music music);
    void LoopMusic(Music music);
    void FadeOutMusic(Music music);
  }
}