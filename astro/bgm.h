#pragma once

namespace astro {
  enum Music {
    MUSIC_NONE = -1,
    DIVINATION_WOODREALM
  };

  namespace BGM {
    void PlayMusic(Music music);
    void LoopMusic(Music music);
    void FadeCurrentMusicVolumeTo(const float volume, uint64 duration);
    void FadeOutMusic(Music music);
  }
}