#pragma once

#include "astro/entity_behaviors/behavior.h"
#include "astro/entity_behaviors/CastleStairs.h"

namespace astro {
  behavior NormalSwitch {
    static void TriggerAssociatedEntity(Tachyon* tachyon, State& state, GameEntity& entity) {
      auto& associated = entity.associated_entity_record;

      if (associated.type == UNSPECIFIED || associated.id == -1) {
        return;
      }

      auto& associated_entity = *EntityManager::FindEntity(state, associated);

      if (!associated_entity.did_activate) {
        // Associated entities can react according to their own rules;
        // we simply mark them as activated here
        associated_entity.did_activate = true;
        associated_entity.game_activation_time = get_scene_time();
        associated_entity.astro_activation_time = state.astro_time;
      }
    }

    static void Press(Tachyon* tachyon, State& state, GameEntity& entity) {
      // Do nothing for pressed switches
      if (entity.did_activate) return;

      entity.did_activate = true;
      entity.game_activation_time = get_scene_time();
      entity.astro_activation_time = state.astro_time;

      TriggerAssociatedEntity(tachyon, state, entity);

      // @todo refactor
      {
        auto r = Tachyon_GetRandom();

        if (r < 0.33f) Sfx::PlaySound(SFX_NORMAL_SWITCH_1, 0.5f);
        else if (r < 0.66f) Sfx::PlaySound(SFX_NORMAL_SWITCH_2, 0.5f);
        else Sfx::PlaySound(SFX_NORMAL_SWITCH_3, 0.5f);
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
    }

    getMeshes() {
      return_meshes({
        meshes.normal_switch_base,
        meshes.normal_switch_button
      });
    }

    getPlaceholderMesh() {
      return meshes.normal_switch_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      reset_instances(meshes.normal_switch_base);
      reset_instances(meshes.normal_switch_button);

      for (auto& entity : state.normal_switches) {
        float dx = abs(state.player_position.x - entity.position.x);
        float dz = abs(state.player_position.z - entity.position.z);

        // Base
        {
          auto& base = use_instance(meshes.normal_switch_base);

          Sync(base, entity);

          base.color = tVec3f(0.4f, 0.2f, 0.1f);
          base.material = tVec4f(0.9f, 0, 0, 0.1f);

          commit(base);
        }

        // Switch
        {
          auto& button = use_instance(meshes.normal_switch_button);

          Sync(button, entity);

          // Pressing behavior
          if (dx < entity.scale.x && dz < entity.scale.z) {
            Press(tachyon, state, entity);

            entity.accumulation_value = Tachyon_Lerpf(entity.accumulation_value, 200.f, 5.f * state.dt);
            button.position.y -= entity.accumulation_value;

          // Unpressing behavior
          } else {
            Unpress(tachyon, state, entity);

            entity.accumulation_value = Tachyon_Lerpf(entity.accumulation_value, 0.f, state.dt);
            button.position.y -= entity.accumulation_value;
          }

          button.color = tVec3f(0.6f, 0.4f, 0.3f);
          button.material = tVec4f(0.9f, 0, 0, 0.1f);

          commit(button);
        }
      }
    }
  };
}
