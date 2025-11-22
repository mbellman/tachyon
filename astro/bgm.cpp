#include <map>

#include "engine/tachyon.h"
#include "astro/bgm.h"

using namespace astro;

static std::map<Music, const char*> music_file_map = {
  { DIVINATION_WOODREALM, "./astro/audio/divination_woodrealm.wav" },
  { VILLAGE_1, "./astro/audio/village1.wav" }
};

static std::map<Music, tSoundResource> music_cache;

static Music current_music = MUSIC_NONE;

static tSoundResource& FindSoundResource(Music music) {
  if (music_cache.find(music) == music_cache.end()) {
    auto file_path = music_file_map.at(music);

    // @todo uninitialize resources at quit, or on demand
    music_cache[music] = Tachyon_CreateSound(file_path);
  }

  return music_cache[music];
}

void BGM::LoopMusic(Music music, const float volume) {
  if (music == current_music) {
    return;
  }

  if (current_music != MUSIC_NONE) {
    // Fade out and stop the previous music
    auto& current_sound = FindSoundResource(current_music);

    Tachyon_FadeOutSound(current_sound, 2000);
    Tachyon_StopSoundAfterDuration(current_sound, 2000);
  }

  auto& resource = FindSoundResource(music);

  Tachyon_LoopSound(resource, volume);

  // @hack Reset the stored volume so we can fade the sound in
  resource.volume = 0.f;

  Tachyon_FadeInSound(resource, volume, 5000);

  current_music = music;
}

void BGM::FadeCurrentMusicVolumeTo(const float volume, uint64 duration) {
  if (current_music == MUSIC_NONE) return;

  auto& resource = FindSoundResource(current_music);

  Tachyon_FadeSoundTo(resource, volume, duration);
}

void BGM::FadeOutMusic(Music music) {
  // @todo
}