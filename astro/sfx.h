#pragma once

namespace astro {
  enum Sound {
    SFX_ASTRO_START,
    SFX_ASTRO_END,
    SFX_ASTRO_BELLS,

    SFX_GROUND_WALK_1,
    SFX_GROUND_WALK_2,
    SFX_GROUND_WALK_3,

    SFX_SPELL_STUN,

    SFX_LIGHT_POST_ACTIVATE,
    SFX_LIGHT_POST_ASTRO_SYNCED
  };

  namespace Sfx {
    void PlaySound(Sound sound, const float volume = 1.f);
    void FadeOutSound(Sound sound, uint64 duration);
  }
}