#include "astro/wand_abilities.h"
#include "astro/sfx.h"

using namespace astro;

static void ShowWandHint(Tachyon* tachyon, State& state, const tVec3f& position) {
  state.last_wand_hint_time = get_scene_time();

  Sfx::PlaySound(SFX_WAND_HINT_1, 1.f);

  // Update light
  {
    if (state.wand_hint_light_id == -1) {
      state.wand_hint_light_id = create_point_light();
    }

    auto& hint_light = *get_point_light(state.wand_hint_light_id);

    hint_light.position = position;
  }
}

void WandAbilities::CheckForHints(Tachyon* tachyon, State& state) {
  if (time_since(state.last_wand_hint_time) < 4.f) {
    return;
  }

  for (auto& entity : state.npcs) {
    float player_distance = tVec3f::distance(entity.position, state.player_position);

    if (
      entity.astro_start_time > state.astro_time &&
      player_distance < 7500.f
    ) {
      ShowWandHint(tachyon, state, entity.position);
    }
  }

  for (auto& entity : state.low_guards) {
    float player_distance = tVec3f::distance(entity.position, state.player_position);

    if (
      entity.astro_start_time > state.astro_time &&
      player_distance < 7500.f
    ) {
      ShowWandHint(tachyon, state, entity.position);
    }
  }
}

void WandAbilities::HandleWandAbilities(Tachyon* tachyon, State& state) {
  // Hint lights
  {
    if (
      state.wand_hint_light_id != -1 &&
      time_since(state.last_wand_hint_time) < t_PI
    ) {
      auto& hint_light = *get_point_light(state.wand_hint_light_id);
      float alpha = sinf(time_since(state.last_wand_hint_time));

      hint_light.power = alpha;
      hint_light.glow_power = alpha;
      hint_light.color = tVec3f(1.f, 0.6f, 0.4f);
      hint_light.radius = 6000.f * alpha;

      hint_light.position.y += 500.f * sinf(get_scene_time()) * state.dt;
    }
  }
}