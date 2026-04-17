#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior MagicGate {
    addMeshes() {
      meshes.magic_gate_placeholder = MODEL_MESH("./astro/3d_models/magic_gate/placeholder.obj", 100);
      meshes.magic_gate = MODEL_MESH("./astro/3d_models/magic_gate/gate.obj", 100);
    }

    getMeshes() {
      return_meshes({
        meshes.magic_gate
      });
    }

    getPlaceholderMesh() {
      return meshes.magic_gate_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      float player_speed = state.player_velocity.magnitude();

      reset_instances(meshes.magic_gate);

      for_entities(state.magic_gates) {
        auto& entity = state.magic_gates[i];

        if (abs(state.player_position.x - entity.position.x) > 25000.f) continue;
        if (abs(state.player_position.z - entity.position.z) > 25000.f) continue;

        // Gate
        {
          auto& gate = use_instance(meshes.magic_gate);

          Sync(gate, entity);

          gate.material = tVec4f(0.9f, 0, 0, 0);

          commit(gate);
        }

        // Collision
        entity.visible_position = entity.position;
        entity.visible_rotation = entity.orientation;
        entity.visible_scale = entity.scale * tVec3f(0.4f, 1.f, 1.4f);
      }
    }
  };
}