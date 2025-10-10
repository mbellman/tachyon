#pragma once

namespace astro {
  enum Sound {
    SFX_ASTRO_START,
    SFX_ASTRO_END
  };

  namespace Sfx {
    void PlaySound(Sound sound);
    void FadeOutSound(Sound sound);
  }
}