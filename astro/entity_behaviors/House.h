#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior House {
    addMeshes() {
      meshes.house_placeholder = MODEL_MESH("./astro/3d_models/house/placeholder.obj", 500);
      meshes.house_body = MODEL_MESH("./astro/3d_models/house/body.obj", 500);
      meshes.house_frame = MODEL_MESH("./astro/3d_models/house/frame.obj", 500);
      meshes.house_door = MODEL_MESH("./astro/3d_models/house/door.obj", 500);
      meshes.house_window_panes = MODEL_MESH("./astro/3d_models/house/window_panes.obj", 500);
      meshes.house_roof = MODEL_MESH("./astro/3d_models/house/roof.obj", 500);
      meshes.house_chimney = MODEL_MESH("./astro/3d_models/house/chimney.obj", 500);
    }

    getMeshes() {
      return_meshes({
        meshes.house_body,
        meshes.house_frame,
        meshes.house_door,
        meshes.house_window_panes,
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

      reset_instances(meshes.house_body);
      reset_instances(meshes.house_frame);
      reset_instances(meshes.house_door);
      reset_instances(meshes.house_window_panes);
      reset_instances(meshes.house_roof);
      reset_instances(meshes.house_chimney);

      for_entities(state.houses) {
        auto& entity = state.houses[i];

        if (!state.use_vantage_camera) {
          if (abs(state.player_position.x - entity.position.x) > 30000.f) continue;
          if (abs(state.player_position.z - entity.position.z) > 30000.f) continue;
        }

        auto& body = use_instance(meshes.house_body);
        auto& frame = use_instance(meshes.house_frame);
        auto& door = use_instance(meshes.house_door);
        auto& window_panes = use_instance(meshes.house_window_panes);
        auto& roof = use_instance(meshes.house_roof);
        auto& chimney = use_instance(meshes.house_chimney);

        Sync(body, entity);
        Sync(frame, entity);
        Sync(door, entity);
        Sync(window_panes, entity);
        Sync(roof, entity);
        Sync(chimney, entity);

        body.color = tVec3f(1.f, 0.9f, 0.8f);
        frame.color = tVec3f(0.6f, 0.4f, 0.3f);
        door.color = tVec3f(0.5f, 0.3f, 0.2f);
        window_panes.color = tVec4f(1.f, 0.8f, 0.6f, 1.f);
        roof.color = tVec3f(0.8f, 0.1f, 0.1f);
        chimney.color = tVec3f(0.6f);

        frame.material = tVec4f(1.f, 0, 0, 0.2f);
        door.material = tVec4f(1.f, 0, 0, 0.1f);

        commit(body);
        commit(frame);
        commit(door);
        commit(window_panes);
        commit(roof);
        commit(chimney);

        // Lights
        {
          if (entity.light_id == -1) {
            entity.light_id = create_point_light();
          }

          auto& light = *get_point_light(entity.light_id);

          light.position = UnitEntityToWorldPosition(entity, tVec3f(1.05f, -0.14f, -0.49f));
          light.radius = 4000.f;
          light.color = tVec3f(1.f, 0.5f, 0.2f);
        }

        // Handle door interactions
        {
          tVec3f door_position = UnitEntityToWorldPosition(entity, tVec3f(1.f, 0, 0.3f));
          float door_distance = tVec3f::distance(state.player_position, door_position);

          if (door_distance < 3500.f && player_speed < 200.f) {
            UISystem::ShowTransientDialogue(tachyon, state, "[X] Knock");

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

        // Collision
        entity.visible_position = entity.position;
        entity.visible_scale = body.scale;
        entity.visible_rotation = entity.orientation;
      }
    }
  };
}