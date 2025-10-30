#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  static float GetSwitchDistance(const tVec3f& player_position, const GameEntity& entity) {
    tVec3f object_space_switch_position = tVec3f(0.4f, 0, 0.65f) * entity.scale;

    tVec3f world_space_switch_position = entity.position + entity.orientation.toMatrix4f() * object_space_switch_position;
    world_space_switch_position.y = player_position.y;

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
          // Handle opening behavior
          float alpha = 0.333f * (tachyon->scene.scene_time - entity.open_time);
          if (alpha < 0.f) alpha = 0.f; // @todo can this happen?
          if (alpha > 1.f) alpha = 1.f;

          tVec3f direction = entity.orientation.toMatrix4f() * tVec3f(0, 0, 1.f);
          float distance = alpha * entity.scale.z * 0.6f;

          door_left.position = entity.position + direction * distance;
          door_right.position = entity.position - direction * distance;
        }

        if (did_press_key(tKey::CONTROLLER_A)) {
          float switch_distance = GetSwitchDistance(state.player_position, entity);

          console_log(switch_distance);

          // Handle switch activation
          if (switch_distance < 1000.f && !entity.is_open) {
            // @todo store astro time
            entity.is_open = true;
            entity.open_time = tachyon->scene.scene_time;
          }
        }

        commit(body);
        commit(door_left);
        commit(door_right);
        commit(gate_switch);
      }
    }
  };
}