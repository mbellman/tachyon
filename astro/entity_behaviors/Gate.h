#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
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
          auto switch_distance = tVec3f::distance(state.player_position, entity.position);

          if (switch_distance < entity.scale.magnitude()) {
            entity.is_open = true;
          }
        }
      }
    }
  };
}