#include "astro/sound_driver.h"
#include "astro/sfx.h"

using namespace astro;

void PlayWalkSound1(State& state, const float volume) {
  if (state.is_on_wood_surface) {
    Sfx::PlaySound(SFX_WOOD_WALK_1, volume);
  } else if (state.is_on_stone_surface) {
    Sfx::PlaySound(SFX_STONE_WALK_1, volume);
  } else {
    Sfx::PlaySound(SFX_GROUND_WALK_1, volume * 0.75f);
  }

  Sfx::PlaySound(SFX_CARRYING_1, volume);
}

void PlayWalkSound2(State& state, const float volume) {
  if (state.is_on_wood_surface) {
    Sfx::PlaySound(SFX_WOOD_WALK_2, volume);
  } else if (state.is_on_stone_surface) {
    Sfx::PlaySound(SFX_STONE_WALK_2, volume);
  } else {
    Sfx::PlaySound(SFX_GROUND_WALK_2, volume * 0.75f);
  }

  Sfx::PlaySound(SFX_CARRYING_2, volume);
}

void PlayWalkSound3(State& state, const float volume) {
  if (state.is_on_wood_surface) {
    Sfx::PlaySound(SFX_WOOD_WALK_3, volume);
  } else if (state.is_on_stone_surface) {
    Sfx::PlaySound(SFX_STONE_WALK_3, volume);
  } else {
    Sfx::PlaySound(SFX_GROUND_WALK_3, volume * 0.75f);
  }

  if (Tachyon_GetRandom() < 0.5f) {
    Sfx::PlaySound(SFX_CARRYING_1, volume);
  } else {
    Sfx::PlaySound(SFX_CARRYING_2, volume);
  }
}

void SoundDriver::PlayWalkSound(State& state, const float volume) {
  state.walk_cycle++;

  if (state.walk_cycle == 1) {
    PlayWalkSound1(state, volume);
  }
  else if (state.walk_cycle == 2) {
    PlayWalkSound2(state, volume);
  }
  else if (state.walk_cycle >= 3) {
    PlayWalkSound3(state, volume);

    state.walk_cycle = 0;
  }
}

void SoundDriver::PlayLadderSound(State& state, const float volume) {
  state.walk_cycle++;

  if (state.walk_cycle == 1) {
    Sfx::PlaySound(SFX_LADDER_WALK_1, volume);
  }
  else if (state.walk_cycle == 2) {
    Sfx::PlaySound(SFX_LADDER_WALK_2, volume);
  }
  else if (state.walk_cycle >= 3) {
    Sfx::PlaySound(SFX_LADDER_WALK_3, volume);

    state.walk_cycle = 0;
  }
}