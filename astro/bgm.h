#pragma once

namespace astro {
  enum Music {
    MUSIC_NONE = -1,
    BGM_STORY,
    BGM_WIND_CHIMES,
    DIVINATION_WOODREALM,
    // @todo rename
    VILLAGE_1
  };

  namespace BGM {
    void LoopMusic(Music music, const float volume);
    void StopCurrentMusic();
    void FadeCurrentMusicVolumeTo(const float volume, uint64 duration);
  }
}