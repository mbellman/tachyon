#pragma once

#include "astro/entity_behaviors/behavior.h"
#include "astro/entity_behaviors/CastleStairs.h"

namespace astro {
  behavior NormalSwitch {
    static int pulse_sound_cycle = 0;

    // @todo refactor
    static bool TriggerAssociatedEntity(Tachyon* tachyon, State& state, GameEntity& entity) {
      auto& associated = entity.associated_entity_record;

      if (associated.type == UNSPECIFIED || associated.id == -1) {
        return false;
      }

      auto& associated_entity = *EntityManager::FindEntity(state, associated);

      if (associated_entity.did_activate) {
        return false;
      }

      // Associated entities can react according to their own rules;
      // we simply mark them as activated here
      associated_entity.did_activate = true;
      associated_entity.game_activation_time = get_scene_time();
      associated_entity.astro_activation_time = state.astro_time;

      return true;
    }

    static void Press(Tachyon* tachyon, State& state, GameEntity& entity) {
      // Do nothing for pressed switches
      if (entity.did_activate) return;

      entity.did_activate = true;
      entity.game_activation_time = get_scene_time();
      entity.astro_activation_time = state.astro_time;

      bool did_trigger_effect = TriggerAssociatedEntity(tachyon, state, entity);

      // @todo refactor
      {
        auto r = Tachyon_GetRandom();

        if (r < 0.33f) Sfx::PlaySound(SFX_NORMAL_SWITCH_1, 0.3f);
        else if (r < 0.66f) Sfx::PlaySound(SFX_NORMAL_SWITCH_2, 0.3f);
        else Sfx::PlaySound(SFX_NORMAL_SWITCH_3, 0.3f);
      }

      if (did_trigger_effect) {
        Sfx::PlaySound(SFX_SWITCH_ACTIVATE, 0.6f);
      } else {
        if (pulse_sound_cycle == 0) {
          Sfx::PlaySound(SFX_SWITCH_PULSE_1, 0.5f);

          pulse_sound_cycle++;
        } else {
          Sfx::PlaySound(SFX_SWITCH_PULSE_2, 0.5f);

          pulse_sound_cycle = 0;
        }
      }
    }

    static void Unpress(Tachyon* tachyon, State& state, GameEntity& entity) {
      // Do nothing for unpressed switches
      if (!entity.did_activate) return;

      entity.did_activate = false;
      entity.game_activation_time = get_scene_time();
      entity.astro_activation_time = -1.f;
    }

    addMeshes() {
      meshes.normal_switch_placeholder = MODEL_MESH("./astro/3d_models/normal_switch/placeholder.obj", 500);
      meshes.normal_switch_base = MODEL_MESH("./astro/3d_models/normal_switch/base.obj", 500);
      meshes.normal_switch_button = MODEL_MESH("./astro/3d_models/normal_switch/button.obj", 500);
      meshes.normal_switch_emblem = MODEL_MESH("./astro/3d_models/normal_switch/emblem.obj", 500);

      mesh(meshes.normal_switch_base).shadow_cascade_ceiling = 2;
      mesh(meshes.normal_switch_button).shadow_cascade_ceiling = 2;
      mesh(meshes.normal_switch_emblem).shadow_cascade_ceiling = 0;
    }

    getMeshes() {
      return_meshes({
        meshes.normal_switch_base,
        meshes.normal_switch_button,
        meshes.normal_switch_emblem
      });
    }

    getPlaceholderMesh() {
      return meshes.normal_switch_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      reset_instances(meshes.normal_switch_base);
      reset_instances(meshes.normal_switch_button);
      reset_instances(meshes.normal_switch_emblem);

      for (auto& entity : state.normal_switches) {
        float dx = abs(state.player_position.x - entity.position.x);
        float dz = abs(state.player_position.z - entity.position.z);

        // Range culling
        if (dx > 25000.f || dz > 25000.f) continue;

        float activation_alpha = entity.accumulation_value;
        float press_down_y = activation_alpha * 200.f;

        // Base
        {
          auto& base = use_instance(meshes.normal_switch_base);

          Sync(base, entity);

          base.color = tVec4f(1.f, 0.6f, 0.2f, activation_alpha);
          base.material = tVec4f(0.5f, 1.f, 0, 0.1f);

          commit(base);
        }

        // Switch
        {
          auto& button = use_instance(meshes.normal_switch_button);
          bool is_force_activated = entity.did_activate && !entity.can_activate;

          Sync(button, entity);

          // Pressing behavior
          if (
            (dx < entity.scale.x && dz < entity.scale.z) ||
            is_force_activated
          ) {
            Press(tachyon, state, entity);

            entity.accumulation_value = Tachyon_Lerpf(entity.accumulation_value, 1.f, 5.f * state.dt);

          // Unpressing behavior
          } else if (entity.can_activate) {
            Unpress(tachyon, state, entity);

            entity.accumulation_value = Tachyon_Lerpf(entity.accumulation_value, 0.f, state.dt);
          }

          button.position.y -= press_down_y;
          button.color = tVec3f(0.8f, 0.6f, 0.5f);
          button.material = tVec4f(0.9f, 0, 0, 0.1f);

          commit(button);
        }

        // Emblem
        {
          auto& emblem = use_instance(meshes.normal_switch_emblem);

          Sync(emblem, entity);

          // Match the button motion
          emblem.position.y -= press_down_y;

          emblem.color = tVec4f(1.f, 0.6f, 0.2f, activation_alpha);
          emblem.material = tVec4f(0.3f, 1.f, 0, 0.1f);

          commit(emblem);
        }
      }
    }
  };
}
