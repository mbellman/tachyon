#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior Npc {
    addMeshes() {
      meshes.npc_placeholder = MODEL_MESH("./astro/3d_models/npc/placeholder.obj", 500);
    }

    getMeshes() {
      return_meshes({});
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

        if (!is_active) continue;

        // Interaction
        {
          float player_distance = tVec3f::distance(state.player_position, entity.visible_position);

          tVec3f unit_player_to_entity = (entity.visible_position - state.player_position).xz().unit();
          float facing_dot = tVec3f::dot(state.player_facing_direction, unit_player_to_entity);

          // Initiating dialogue
          if (
            player_distance < 4000.f &&
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

              entity.did_activate = true;
              entity.game_activation_time = get_scene_time();

              UISystem::StartDialogueSet(state, entity.unique_name);
            }
          }
        }

        // Turn to face the player upon interaction
        {
          bool did_just_interact = (
            entity.did_activate &&
            time_since(entity.game_activation_time) < 1.f
          );

          if (did_just_interact) {
            FacePlayer(entity, state);
          }
        }
      }
    }
  };
}