#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior IronGate {
    addMeshes() {
      meshes.iron_gate_placeholder = MODEL_MESH("./astro/3d_models/iron_gate/placeholder.obj", 100);
      meshes.iron_gate_side = MODEL_MESH("./astro/3d_models/iron_gate/gate.obj", 200);
      meshes.iron_gate_wall = MODEL_MESH("./astro/3d_models/iron_gate/wall.obj", 100);

      mesh(meshes.iron_gate_side).shadow_cascade_ceiling = 2;
      mesh(meshes.iron_gate_wall).shadow_cascade_ceiling = 2;
    }

    getMeshes() {
      return_meshes({
        // Add each side of the gate
        meshes.iron_gate_side,
        meshes.iron_gate_side,

        // Wall
        meshes.iron_gate_wall
      });
    }

    getPlaceholderMesh() {
      return meshes.iron_gate_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      reset_instances(meshes.iron_gate_side);
      reset_instances(meshes.iron_gate_wall);

      for_entities(state.iron_gates) {
        auto& entity = state.iron_gates[i];

        if (!IsDuringActiveTime(entity, state)) continue;
        if (abs(state.player_position.x - entity.position.x) > 25000.f) continue;
        if (abs(state.player_position.z - entity.position.z) > 25000.f) continue;

        entity.visible_position = entity.position;
        entity.visible_rotation = entity.orientation;
        entity.visible_scale = entity.scale;

        // Opening the gate
        const float opening_duration = 2.5f;
        float open_alpha = 0.f;

        {
          if (entity.did_activate) {
            open_alpha = time_since(entity.game_activation_time) / opening_duration;

            clamp_to_1(open_alpha);

            open_alpha = Tachyon_EaseInOutf(open_alpha);
          }
        }

        // @temporary
        // @todo implement proper opening triggers
        if (
          abs(state.player_position.x - entity.position.x) < 2000.f &&
          abs(state.player_position.z - entity.position.z) < 2000.f &&
          !entity.did_activate
        ) {
          entity.did_activate = true;
          entity.game_activation_time = get_scene_time();
        }

        // Gate (left)
        {
          auto& left_gate = use_instance(meshes.iron_gate_side);

          Sync(left_gate, entity);

          left_gate.position = UnitEntityToWorldPosition(entity, tVec3f(0.475f, 0, 0));
          left_gate.material = tVec4f(0.3f, 1.f, 0, 0);

          // Handle opening animation
          left_gate.rotation = Quaternion::slerp(
            left_gate.rotation,
            left_gate.rotation * Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), -t_HALF_PI),
            open_alpha
          );

          commit(left_gate);
        }

        // Gate (right)
        {
          auto& right_gate = use_instance(meshes.iron_gate_side);

          Sync(right_gate, entity);

          right_gate.position = UnitEntityToWorldPosition(entity, tVec3f(-0.475f, 0, 0));
          right_gate.rotation *= Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), t_PI);
          right_gate.material = tVec4f(0.3f, 1.f, 0, 0);

          // Handle opening animation
          right_gate.rotation = Quaternion::slerp(
            right_gate.rotation,
            right_gate.rotation * Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), t_HALF_PI),
            open_alpha
          );

          // right_gate.color = 0xCC5F;
          // right_gate.material = tVec4f(0.f, 1.f, 1.f, 1.f);

          commit(right_gate);
        }

        // Wall
        {
          auto& wall = use_instance(meshes.iron_gate_wall);

          Sync(wall, entity);

          wall.color = tVec3f(0.5f);
          wall.material = tVec4f(0.8f, 0, 0, 0);

          commit(wall);
        }
      }
    }
  };
}