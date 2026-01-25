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

        bool is_active = IsDuringActiveTime(entity, state);

        // Body
        {
          auto& body = objects(meshes.npc)[i];

          Sync(body, entity);

          body.position = entity.visible_position;
          body.rotation = entity.visible_rotation;

          if (!is_active) {
            // Make invisible during inactive times
            body.scale = tVec3f(0.f);
          }

          commit(body);
        }

        // Interaction
        {
          float player_distance = tVec3f::distance(state.player_position, entity.visible_position);

          tVec3f unit_player_to_entity = (entity.visible_position - state.player_position).xz().unit();
          float facing_dot = tVec3f::dot(state.player_facing_direction, unit_player_to_entity);

          // Initiating dialogue
          if (
            is_active &&
            player_distance < 3000.f &&
            player_speed < 200.f &&
            facing_dot > 0.5f
          ) {
            UISystem::ShowTransientDialogue(tachyon, state, "[X] Speak");

            if (
              did_press_key(tKey::CONTROLLER_A) &&
              !state.has_blocking_dialogue
            ) {
              // Reset player speed
              state.player_velocity = tVec3f(0.f);

              UISystem::StartDialogueSet(state, entity.unique_name);
            }
          }
        }
      }
    }
  };
}