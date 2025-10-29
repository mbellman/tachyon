#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  static float GetSwitchDistance(const tVec3f& player_position, const GameEntity& entity) {
    tVec3f object_space_switch_position = tVec3f(0, 0, 0.6f) * entity.scale;
    tVec3f world_space_switch_position = entity.position + entity.orientation.toMatrix4f() * object_space_switch_position;

    return tVec3f::distance(player_position, world_space_switch_position);
  }

  behavior Gate {
    addMeshes() {
      meshes.gate_placeholder = MODEL_MESH("./astro/3d_models/gate/placeholder.obj", 500);
      meshes.gate_body = MODEL_MESH("./astro/3d_models/gate/body.obj", 500);
      meshes.gate_left_door = MODEL_MESH("./astro/3d_models/gate/left_door.obj", 500);
      meshes.gate_right_door = MODEL_MESH("./astro/3d_models/gate/right_door.obj", 500);
      meshes.gate_switch = MODEL_MESH("./astro/3d_models/gate/switch.obj", 500);
    }

    getMeshes() {
      return_meshes({
        meshes.gate_body,
        meshes.gate_left_door,
        meshes.gate_right_door,
        meshes.gate_switch
      });
    }

    getPlaceholderMesh() {
      return meshes.gate_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      const float lifetime = 100.f;

      for_entities(state.gates) {
        auto& entity = state.gates[i];

        auto& body = objects(meshes.gate_body)[i];
        auto& door_left = objects(meshes.gate_left_door)[i];
        auto& door_right = objects(meshes.gate_right_door)[i];
        auto& gate_switch = objects(meshes.gate_switch)[i];

        Sync(body, entity);
        Sync(door_left, entity);
        Sync(door_right, entity);
        Sync(gate_switch, entity);

        door_left.material = tVec4f(0.2f, 1.f, 0, 0);
        door_right.material = tVec4f(0.2f, 1.f, 0, 0);

        gate_switch.color = tVec4f(1.f, 0.6f, 0.2f, 0.1f);
        gate_switch.material = tVec4f(1.f, 0, 0, 0.1f);

        if (entity.is_open) {
          // @temporary
          door_left.scale = tVec3f(0.f);
          door_right.scale = tVec3f(0.f);
        }

        commit(body);
        commit(door_left);
        commit(door_right);
        commit(gate_switch);

        if (did_press_key(tKey::CONTROLLER_A)) {
          float switch_distance = GetSwitchDistance(state.player_position, entity);

          if (switch_distance < 3000.f) {
            entity.is_open = true;
          }
        }
      }
    }
  };
}