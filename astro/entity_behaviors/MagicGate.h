#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior MagicGate {
    addMeshes() {
      meshes.magic_gate_placeholder = MODEL_MESH("./astro/3d_models/magic_gate/placeholder.obj", 100);
      meshes.magic_gate = MODEL_MESH("./astro/3d_models/magic_gate/gate.obj", 100);
      meshes.magic_gate_barrier = CUBE_MESH(100);

      // @temporary
      // @todo make a new shader for this
      mesh(meshes.magic_gate_barrier).type = FIRE_MESH;
    }

    getMeshes() {
      return_meshes({
        meshes.magic_gate,
        meshes.magic_gate_barrier
      });
    }

    getPlaceholderMesh() {
      return meshes.magic_gate_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      float player_speed = state.player_velocity.magnitude();

      reset_instances(meshes.magic_gate);
      reset_instances(meshes.magic_gate_barrier);

      // @temporary
      tVec3f barrier_color = tVec3f(0.3f, 0.2f, 1.f);

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

        // Barrier
        {
          auto& barrier = use_instance(meshes.magic_gate_barrier);

          Sync(barrier, entity);

          barrier.position.y -= entity.scale.y * 0.1f;
          barrier.scale.x *= 0.05f;
          barrier.scale.z *= 0.5f;
          barrier.color = barrier_color;

          commit(barrier);
        }

        // Barrier light
        {
          if (entity.light_id == -1) {
            entity.light_id = create_point_light();
          }

          auto& light = *get_point_light(entity.light_id);

          light.position = entity.position;
          light.color = barrier_color;
          light.radius = 8000.f;
          light.power = 3.f;
          light.glow_power = 0.f;
        }

        // Collision
        entity.visible_position = entity.position;
        entity.visible_rotation = entity.orientation;
        entity.visible_scale = entity.scale * tVec3f(0.4f, 1.f, 1.4f);
      }
    }
  };
}