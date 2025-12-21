#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior House {
    addMeshes() {
      meshes.house_placeholder = MODEL_MESH("./astro/3d_models/house/placeholder.obj", 500);
      meshes.house_body = MODEL_MESH("./astro/3d_models/house/body.obj", 500);
      meshes.house_frame = MODEL_MESH("./astro/3d_models/house/frame.obj", 500);
      meshes.house_roof = MODEL_MESH("./astro/3d_models/house/roof.obj", 500);
      meshes.house_chimney = MODEL_MESH("./astro/3d_models/house/chimney.obj", 500);
    }

    getMeshes() {
      return_meshes({
        meshes.house_body,
        meshes.house_frame,
        meshes.house_roof,
        meshes.house_chimney
      });
    }

    getPlaceholderMesh() {
      return meshes.house_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;
      const float lifetime = 100.f;

      float player_speed = state.player_velocity.magnitude();

      for_entities(state.houses) {
        auto& entity = state.houses[i];

        auto& body = objects(meshes.house_body)[i];
        auto& frame = objects(meshes.house_frame)[i];
        auto& roof = objects(meshes.house_roof)[i];
        auto& chimney = objects(meshes.house_chimney)[i];

        Sync(body, entity);
        Sync(frame, entity);
        Sync(roof, entity);
        Sync(chimney, entity);

        body.color = entity.tint;
        frame.color = tVec3f(0.6f, 0.4f, 0.3f);
        roof.color = tVec3f(0.5f, 0.4f, 0.3f);
        chimney.color = tVec3f(0.6f);

        commit(body);
        commit(frame);
        commit(roof);
        commit(chimney);

        // Handle door interactions
        {
          tVec3f door_position = UnitEntityToWorldPosition(entity, tVec3f(1.f, 0, 0.3f));
          float door_distance = tVec3f::distance(state.player_position, door_position);

          if (door_distance < 3500.f && player_speed < 200.f) {
            UISystem::ShowTransientDialogue(tachyon, state, "[X] Knock");

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

        // Collision
        entity.visible_position = entity.position;
        entity.visible_scale = body.scale;
        entity.visible_rotation = entity.orientation;
      }
    }
  };
}