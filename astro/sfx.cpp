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

  { SFX_SWORD_DAMAGE, "./astro/audio/sfx_sword_damage.wav" },
  { SFX_WAND_SWING, "./astro/audio/sfx_wand_swing.wav" },

  { SFX_SPELL_STUN, "./astro/audio/sfx_spell_stun.wav" },

  { SFX_LIGHT_POST_ACTIVATE, "./astro/audio/light_post.wav" },
  { SFX_LIGHT_POST_ASTRO_SYNCED, "./astro/audio/light_post_synced.wav" },
  { SFX_LIGHT_POST_ASTRO_SYNCED_2, "./astro/audio/light_post_synced_2.wav" },

  { SFX_FOREST, "./astro/audio/sfx_forest.wav" },
  { SFX_FOREST_NIGHT, "./astro/audio/sfx_forest_night.wav" }
};

static std::map<Sound, tSoundResource> resource_cache;

static Sound current_looping_sound = SFX_NONE;

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
  if (sound == current_looping_sound) {
    return;
  }

  if (current_looping_sound != SFX_NONE) {
    // Fade out and stop the previous sound
    auto& current_sound = FindSoundResource(current_looping_sound);

    Tachyon_FadeOutSound(current_sound, 2000);
    Tachyon_StopSoundAfterDuration(current_sound, 2000);
  }

  auto& resource = FindSoundResource(sound);

  Tachyon_LoopSound(resource, volume);

  // @hack Reset the stored volume so we can fade the sound in
  resource.volume = 0.f;

  Tachyon_FadeInSound(resource, volume, 5000);

  current_looping_sound = sound;
}

void Sfx::FadeSoundVolumeTo(Sound sound, const float volume, uint64 duration) {
  auto& resource = FindSoundResource(sound);

  Tachyon_FadeSoundTo(resource, volume, duration);
}

void Sfx::FadeOutSound(Sound sound, uint64 duration) {
  auto& resource = FindSoundResource(sound);

  Tachyon_FadeSoundTo(resource, 0.f, duration);
}