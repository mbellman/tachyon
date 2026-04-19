#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior BirdGate {
    static Quaternion Y_FLIP = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), t_PI);

    addMeshes() {
      meshes.bird_gate_placeholder = MODEL_MESH("./astro/3d_models/bird_gate/placeholder.obj", 500);
      meshes.bird_gate = MODEL_MESH("./astro/3d_models/bird_gate/gate_half.obj", 500);
    }

    getMeshes() {
      return_meshes({
        // Left half
        meshes.bird_gate,
        // Right half
        meshes.bird_gate
      });
    }

    getPlaceholderMesh() {
      return meshes.bird_gate_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      float player_speed = state.player_velocity.magnitude();

      reset_instances(meshes.bird_gate);

      for_entities(state.bird_gates) {
        auto& entity = state.bird_gates[i];

        if (abs(state.player_position.x - entity.position.x) > 25000.f) continue;
        if (abs(state.player_position.z - entity.position.z) > 25000.f) continue;

        // Gate (left)
        {
          auto& gate = use_instance(meshes.bird_gate);

          Sync(gate, entity);

          gate.color = tVec3f(1.f, 0.9f, 0.2f);
          gate.material = tVec4f(0.f, 1.f, 0, 0.6f);

          commit(gate);
        }

        // Gate (right)
        {
          auto& gate = use_instance(meshes.bird_gate);

          Sync(gate, entity);

          gate.color = tVec3f(1.f, 0.9f, 0.2f);
          gate.material = tVec4f(0.f, 1.f, 0, 0.6f);
          gate.rotation *= Y_FLIP;

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