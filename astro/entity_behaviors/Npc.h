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

          // Initiating dialogue
          if (player_distance < 3000.f && player_speed < 200.f) {
            UISystem::ShowTransientDialogue(tachyon, state, "[X] Speak");

            if (
              !state.has_blocking_dialogue &&
              did_press_key(tKey::CONTROLLER_A)
            ) {
              // Reset player speed
              state.player_velocity = tVec3f(0.f);

              // Start dialogue
              state.current_dialogue_set = entity.unique_name;
              state.current_dialogue_step = 0;
            }
          }
        }
      }
    }
  };
}