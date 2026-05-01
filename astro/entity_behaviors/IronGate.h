#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior IronGate {
    addMeshes() {
      meshes.iron_gate_placeholder = MODEL_MESH("./astro/3d_models/iron_gate/placeholder.obj", 100);
      meshes.iron_gate_side = MODEL_MESH("./astro/3d_models/iron_gate/gate.obj", 200);
    }

    getMeshes() {
      return_meshes({
        // Add each side of the gate
        meshes.iron_gate_side,
        meshes.iron_gate_side
      });
    }

    getPlaceholderMesh() {
      return meshes.iron_gate_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      reset_instances(meshes.iron_gate_side);

      for_entities(state.iron_gates) {
        auto& entity = state.iron_gates[i];

        if (abs(state.player_position.x - entity.position.x) > 25000.f) continue;
        if (abs(state.player_position.z - entity.position.z) > 25000.f) continue;

        auto& fade_out = entity.accumulation_value;

        // Gate (left)
        {
          auto& left_gate = use_instance(meshes.iron_gate_side);

          Sync(left_gate, entity);

          left_gate.material = tVec4f(0.3f, 1.f, 0, 0);

          commit(left_gate);
        }

        // Gate (right)
        {
          auto& right_gate = use_instance(meshes.iron_gate_side);

          Sync(right_gate, entity);

          right_gate.rotation *= Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), t_PI);
          right_gate.material = tVec4f(0.3f, 1.f, 0, 0);

          commit(right_gate);
        }

        // Collision
        entity.visible_position = entity.position;
        entity.visible_rotation = entity.orientation;
        entity.visible_scale = entity.scale * tVec3f(0.4f, 1.f, 1.4f);
      }
    }
  };
}