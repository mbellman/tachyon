#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  // @todo factor
  static tVec3f GetWorldSpaceSwitchPosition(const GameEntity& entity) {
    const tVec3f model_space_position = tVec3f(0.12f, -0.25f, 0.672f);
    tVec3f object_space_position = model_space_position * entity.scale;

    return entity.position + entity.orientation.toMatrix4f() * object_space_position;
  }

  // @todo factor
  static tVec3f GetWorldSpaceActivationPosition(const GameEntity& entity) {
    const tVec3f model_space_position = tVec3f(0.4f, 0, 0.65f);
    tVec3f object_space_position = model_space_position * entity.scale;

    return entity.position + entity.orientation.toMatrix4f() * object_space_position;
  }

  behavior Gate {
    addMeshes() {
      meshes.gate_placeholder = MODEL_MESH("./astro/3d_models/gate/placeholder.obj", 500);
      meshes.gate_body = MODEL_MESH("./astro/3d_models/gate/body.obj", 500);
      meshes.gate_left_door = MODEL_MESH("./astro/3d_models/gate/left_door.obj", 500);
      meshes.gate_right_door = MODEL_MESH("./astro/3d_models/gate/right_door.obj", 500);
      meshes.gate_switch = MODEL_MESH("./astro/3d_models/gate/switch.obj", 500);
      meshes.gate_switch_handle = MODEL_MESH("./astro/3d_models/gate/switch_handle.obj", 500);
    }

    getMeshes() {
      return_meshes({
        meshes.gate_body,
        meshes.gate_left_door,
        meshes.gate_right_door,
        meshes.gate_switch,
        meshes.gate_switch_handle
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
        auto& switch_handle = objects(meshes.gate_switch_handle)[i];

        Sync(body, entity);
        Sync(door_left, entity);
        Sync(door_right, entity);
        Sync(gate_switch, entity);
        Sync(switch_handle, entity);

        door_left.material = tVec4f(0.2f, 1.f, 0, 0);
        door_right.material = tVec4f(0.2f, 1.f, 0, 0);

        gate_switch.color = tVec4f(0.8f, 0.4f, 0.3f, 0.1f);
        gate_switch.material = tVec4f(1.f, 0, 0, 0.1f);

        switch_handle.position = GetWorldSpaceSwitchPosition(entity);
        switch_handle.color = tVec4f(0.8f, 0.2f, 0.1f, 0.2f);
        switch_handle.material = tVec4f(0.6f, 1.f, 0, 0);

        if (entity.is_open) {
          float time_since_opened = tachyon->scene.scene_time - entity.open_time;

          // Rotate the handle
          {
            float rotation_alpha = time_since_opened;
            if (rotation_alpha > 1.f) rotation_alpha = 1.f;
            rotation_alpha = Tachyon_EaseInOutf(rotation_alpha);

            float handle_angle = rotation_alpha * -t_HALF_PI;
            auto handle_rotation = Quaternion::fromAxisAngle(tVec3f(0, 0, 1.f), handle_angle);

            switch_handle.rotation = entity.orientation * handle_rotation;
          }

          // Open the doors
          {
            float open_alpha = time_since_opened * 0.333f - 0.333f;
            if (open_alpha < 0.f) open_alpha = 0.f;
            if (open_alpha > 1.f) open_alpha = 1.f;

            tVec3f direction = entity.orientation.toMatrix4f() * tVec3f(0, 0, 1.f);
            float distance = open_alpha * entity.scale.z * 0.6f;

            door_left.position = entity.position + direction * distance;
            door_right.position = entity.position - direction * distance;
          }
        }

        if (did_press_key(tKey::CONTROLLER_A) && !entity.is_open) {
          tVec3f activation_position = GetWorldSpaceActivationPosition(entity);
          float activation_distance = tVec3f::distance(state.player_position.xz(), activation_position.xz());

          if (activation_distance < 1000.f) {
            // @todo store astro time
            entity.is_open = true;
            entity.open_time = tachyon->scene.scene_time;
          }
        }

        commit(body);
        commit(door_left);
        commit(door_right);
        commit(gate_switch);
        commit(switch_handle);
      }
    }
  };
}