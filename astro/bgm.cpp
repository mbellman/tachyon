#include <map>

#include "engine/tachyon.h"
#include "astro/bgm.h"

using namespace astro;

static std::map<Music, const char*> music_file_map = {
  { DIVINATION_WOODREALM, "./astro/audio/divination_woodrealm.wav" }
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

void BGM::PlayMusic(Music music) {
  auto& resource = FindSoundResource(music);

  Tachyon_PlaySound(resource, 1.f);
}

void BGM::LoopMusic(Music music) {
  if (music == current_music) {
    return;
  }

  if (current_music != MUSIC_NONE) {
    auto& current = FindSoundResource(current_music);

    Tachyon_StopSound(current);
  }

  auto& resource = FindSoundResource(music);

  current_music = music;

  // @todo make volume configurable
  Tachyon_LoopSound(resource, 0.4f);
  Tachyon_FadeInSound(resource, 5000, 0.4f);
}

void BGM::SetCurrentMusicVolume(const float volume) {
  if (current_music == MUSIC_NONE) {
    return;
  }

  auto& resource = FindSoundResource(current_music);

  Tachyon_FadeSoundTo(resource, 500, volume);
}

void BGM::FadeOutMusic(Music music) {
  // @todo
}