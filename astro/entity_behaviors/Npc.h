#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior Npc {
    addMeshes() {
      meshes.npc_placeholder = MODEL_MESH("./astro/3d_models/guy.obj", 500);
      meshes.npc = MODEL_MESH("./astro/3d_models/guy.obj", 500);
    }

    getMeshes() {
      // Path nodes don't have a specific in-game object;
      // path segments are generated between them.
      return_meshes({
        meshes.npc
      });
    }

    getPlaceholderMesh() {
      return meshes.npc_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      float player_speed = state.player_velocity.magnitude();

      for_entities(state.npcs) {
        auto& entity = state.npcs[i];

        // Body
        {
          auto& body = objects(meshes.npc)[0];

          Sync(body, entity);

          commit(body);
        }

        // Interaction
        {
          float player_distance = tVec3f::distance(state.player_position, entity.visible_position);

          // Starting dialogue
          if (player_distance < 3000.f && player_speed < 200.f) {
            UISystem::ShowTransientDialogue(tachyon, state, "[X] Speak");

            if (
              !state.has_blocking_dialogue &&
              did_press_key(tKey::CONTROLLER_A)
            ) {
              // Reset player speed
              state.player_velocity = tVec3f(0.f);

              // Prepare dialogue sequence
              state.current_dialogue_sequence = entity.unique_name;
              state.current_dialogue_step = 0;

              continue;
            }
          }

          // Stepping through dialogue sequence
          if (state.current_dialogue_sequence != "") {
            auto& dialogue_sequence = state.npc_dialogue[entity.unique_name];

            if (state.current_dialogue_step > dialogue_sequence.size() - 1) {
              // Sequence completed
              state.current_dialogue_sequence = "";
              state.current_dialogue_step = 0;

              continue;
            }

            auto& current_dialogue_line = dialogue_sequence[state.current_dialogue_step];

            UISystem::ShowBlockingDialogue(tachyon, state, current_dialogue_line);

            if (did_press_key(tKey::CONTROLLER_A)) {
              state.current_dialogue_step++;
            }
          }
        }
      }
    }
  };
}