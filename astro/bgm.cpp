#include <map>

#include "engine/tachyon.h"
#include "astro/bgm.h"

using namespace astro;

static std::map<Music, const char*> music_file_map = {
  { DIVINATION_WOODREALM, "./astro/audio/divination_woodrealm.wav" }
};

static std::map<Music, tSoundResource> music_cache;

static tSoundResource FindSoundResource(Music music) {
  if (music_cache.find(music) == music_cache.end()) {
    auto file_path = music_file_map.at(music);

    // @todo uninitialize resources at quit, or on demand
    music_cache[music] = Tachyon_CreateSound(file_path);
  }

  return music_cache[music];
}

void BGM::PlayMusic(Music music) {
  auto resource = FindSoundResource(music);

  Tachyon_PlaySound(resource, 1.f);
}

void BGM::LoopMusic(Music music) {
  auto resource = FindSoundResource(music);

  // @todo stop current music
  // @todo set as new current music

  // @todo make volume configurable
  Tachyon_LoopSound(resource, 0.5f);
}

void BGM::FadeOutMusic(Music music) {
  // @todo
}