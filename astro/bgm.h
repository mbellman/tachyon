#pragma once

namespace astro {
  enum Music {
    MUSIC_NONE = -1,
    DIVINATION_WOODREALM,
    // @todo rename
    VILLAGE_1
  };

  namespace BGM {
    void LoopMusic(Music music, const float volume);
    void FadeCurrentMusicVolumeTo(const float volume, uint64 duration);
    void FadeOutMusic(Music music);
  }
}