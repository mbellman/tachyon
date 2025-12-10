#pragma once

namespace astro {
  enum Sound {
    SFX_NONE,

    SFX_ASTRO_BEGIN,
    SFX_ASTRO_TRAVEL,
    SFX_ASTRO_END,
    SFX_ASTRO_DISABLED,

    SFX_GROUND_WALK_1,
    SFX_GROUND_WALK_2,
    SFX_GROUND_WALK_3,

    SFX_SWORD_DAMAGE,
    SFX_WAND_SWING,
    SFX_WAND_HIT,

    SFX_SPELL_STUN,

    SFX_LIGHT_POST_ACTIVATE,
    SFX_LIGHT_POST_ASTRO_SYNCED,
    SFX_LIGHT_POST_ASTRO_SYNCED_2,

    SFX_FOREST,
    SFX_FOREST_NIGHT
  };

  namespace Sfx {
    void PlaySound(Sound sound, const float volume = 1.f);
    void LoopSound(Sound sound, const float volume = 1.f);
    void FadeSoundVolumeTo(Sound sound, const float volume, uint64 duration);
    void FadeOutSound(Sound sound, uint64 duration);
  }
}