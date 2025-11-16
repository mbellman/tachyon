#pragma once

#include "astro/entity_behaviors/behavior.h"
#include "astro/items.h"
#include "astro/ui_system.h"

namespace astro {
  behavior Gate {
    addMeshes() {
      meshes.gate_placeholder = MODEL_MESH("./astro/3d_models/gate/placeholder.obj", 500);
      meshes.gate_body = MODEL_MESH("./astro/3d_models/gate/body.obj", 500);
      meshes.gate_left_door = MODEL_MESH("./astro/3d_models/gate/left_door.obj", 500);
      meshes.gate_right_door = MODEL_MESH("./astro/3d_models/gate/right_door.obj", 500);
      meshes.gate_lock = MODEL_MESH("./astro/3d_models/gate/lock.obj", 500);
      // meshes.gate_switch = MODEL_MESH("./astro/3d_models/gate/switch.obj", 500);
      // meshes.gate_switch_handle = MODEL_MESH("./astro/3d_models/gate/switch_handle.obj", 500);
    }

    getMeshes() {
      return_meshes({
        meshes.gate_body,
        meshes.gate_left_door,
        meshes.gate_right_door,
        meshes.gate_lock
        // meshes.gate_switch,
        // meshes.gate_switch_handle
      });
    }

    getPlaceholderMesh() {
      return meshes.gate_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      const float lifetime = 100.f;

      float player_speed = state.player_velocity.magnitude();

      for_entities(state.gates) {
        auto& entity = state.gates[i];

        tVec3f interaction_position = UnitEntityToWorldPosition(entity, tVec3f(0.4f, -0.2f, 0));
        tVec3f player_to_interaction_position = interaction_position.xz() - state.player_position.xz();
        float distance_from_interaction_position = player_to_interaction_position.magnitude();
        tVec3f interaction_direction = player_to_interaction_position / distance_from_interaction_position;

        auto& body = objects(meshes.gate_body)[i];
        auto& door_left = objects(meshes.gate_left_door)[i];
        auto& door_right = objects(meshes.gate_right_door)[i];
        auto& lock = objects(meshes.gate_lock)[i];

        Sync(body, entity);
        Sync(door_left, entity);
        Sync(door_right, entity);
        Sync(lock, entity);

        body.material = tVec4f(0.9f, 0, 0, 0);

        door_left.material = tVec4f(0.2f, 1.f, 0, 0);
        door_right.material = tVec4f(0.2f, 1.f, 0, 0);

        lock.position = UnitEntityToWorldPosition(entity, tVec3f(0.2f, -0.2f, 0));
        lock.scale = entity.scale * 0.5f;
        lock.color = tVec3f(1.f, 1.f, 0.2f);
        lock.material = tVec4f(0.2f, 1.f, 0, 0.4f);

        bool is_open = (
          entity.game_activation_time > -1.f &&
          state.astro_time >= entity.astro_activation_time
        );

        if (is_open) {
          float time_since_opened = tachyon->scene.scene_time - entity.game_activation_time;

          // Drop the lock
          {
            float lock_alpha = time_since_opened;
            if (lock_alpha > 1.f) lock_alpha = 1.f;
            lock_alpha *= lock_alpha;

            lock.position -= tVec3f(0, lock_alpha * entity.scale.y, 0);
          }

          // Open the doors
          {
            float open_alpha = time_since_opened - 0.5f;
            if (open_alpha < 0.f) open_alpha = 0.f;
            if (open_alpha > 1.f) open_alpha = 1.f;
            open_alpha = Tachyon_EaseInOutf(open_alpha);

            tVec3f direction = entity.orientation.toMatrix4f() * tVec3f(0, 0, 1.f);
            float distance = open_alpha * entity.scale.z * 0.6f;

            door_left.position = entity.position + direction * distance;
            door_right.position = entity.position - direction * distance;
          }
        } else if (
          distance_from_interaction_position < 1500.f &&
          player_speed < 250.f &&
          tVec3f::dot(state.player_facing_direction, interaction_direction) > 0.5f
        ) {
          bool has_gate_key = Items::HasItem(state, GATE_KEY);

          if (did_press_key(tKey::CONTROLLER_A)) {
            if (has_gate_key) {
              entity.game_activation_time = tachyon->scene.scene_time;
              entity.astro_activation_time = state.astro_time;

              UISystem::ShowDialogue(tachyon, state, "The gate was unlocked.");
            } else if (!state.has_blocking_dialogue) {
              // @todo check to see if gate is rusted over
              UISystem::ShowBlockingDialogue(tachyon, state, "The gate is locked.");
            }
          } else if (has_gate_key) {
            UISystem::ShowDialogue(tachyon, state, "[X] Unlock");
          } else {
            UISystem::ShowDialogue(tachyon, state, "[X] Check Gate");
          }
        }

        entity.visible_position = entity.position;
        entity.visible_rotation = entity.orientation;

        // For collision handling
        entity.visible_scale = entity.scale * tVec3f(0.4f, 1.f, 1.4f);

        commit(body);
        commit(door_left);
        commit(door_right);
        commit(lock);
      }
    }
  };
}