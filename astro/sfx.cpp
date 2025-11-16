#include <map>

#include "engine/tachyon.h"
#include "astro/sfx.h"

using namespace astro;

static std::map<Sound, const char*> sound_file_map = {
  { SFX_ASTRO_BEGIN, "./astro/audio/astro_begin.wav" },
  { SFX_ASTRO_TRAVEL, "./astro/audio/astro_travel.wav" },
  { SFX_ASTRO_END, "./astro/audio/astro_end.wav" },
  { SFX_ASTRO_DISABLED, "./astro/audio/astro_disabled.wav" },

  { SFX_GROUND_WALK_1, "./astro/audio/ground_walk_1.wav" },
  { SFX_GROUND_WALK_2, "./astro/audio/ground_walk_2.wav" },
  { SFX_GROUND_WALK_3, "./astro/audio/ground_walk_3.wav" },

  { SFX_SPELL_STUN, "./astro/audio/spell_stun.wav" },

  { SFX_LIGHT_POST_ACTIVATE, "./astro/audio/light_post.wav" },
  { SFX_LIGHT_POST_ASTRO_SYNCED, "./astro/audio/light_post_synced.wav" },
  { SFX_LIGHT_POST_ASTRO_SYNCED_2, "./astro/audio/light_post_synced_2.wav" },

  { SFX_FOREST, "./astro/audio/sfx_forest.wav" }
};

static std::map<Sound, tSoundResource> resource_cache;

static tSoundResource& FindSoundResource(Sound sound) {
  if (resource_cache.find(sound) == resource_cache.end()) {
    auto& file_path = sound_file_map.at(sound);

    // @todo uninitialize resources at quit, or on demand
    resource_cache[sound] = Tachyon_CreateSound(file_path);
  }

  return resource_cache[sound];
}

void Sfx::PlaySound(Sound sound, const float volume) {
  auto& resource = FindSoundResource(sound);

  Tachyon_PlaySound(resource, volume);
}

void Sfx::LoopSound(Sound sound, const float volume) {
  auto& resource = FindSoundResource(sound);

  Tachyon_LoopSound(resource, volume);
}

void Sfx::FadeSoundVolumeTo(Sound sound, const float volume, uint64 duration) {
  auto& resource = FindSoundResource(sound);

  Tachyon_FadeSoundTo(resource, volume, duration);
}

void Sfx::FadeOutSound(Sound sound, uint64 duration) {
  auto& resource = FindSoundResource(sound);

  Tachyon_FadeSoundTo(resource, 0.f, duration);
}