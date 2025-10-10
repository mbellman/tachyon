#include <map>

#include "engine/tachyon.h"
#include "astro/sfx.h"

using namespace astro;

static std::map<Sound, const char*> sound_file_map = {
  { SFX_ASTRO_START, "./astro/audio/astro_start.wav" },
  { SFX_ASTRO_END, "./astro/audio/astro_end.wav" }
};

static std::map<Sound, tSoundResource> resource_cache;

static tSoundResource FindSoundResource(Sound sound) {
  if (resource_cache.find(sound) == resource_cache.end()) {
    auto file_path = sound_file_map.at(sound);

    // @todo uninitialize resources at quit, or on demand
    resource_cache[sound] = Tachyon_CreateSound(file_path);
  }

  return resource_cache[sound];
}

void Sfx::PlaySound(Sound sound) {
  auto resource = FindSoundResource(sound);

  Tachyon_PlaySound(resource);
}

void Sfx::FadeOutSound(Sound sound) {
  auto resource = FindSoundResource(sound);

  Tachyon_FadeOutSound(resource);
}