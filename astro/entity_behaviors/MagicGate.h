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

        auto& fade_out = entity.accumulation_value;

        // Interaction
        {
          auto proximity = GetEntityProximity(entity, state);

          if (
            proximity.distance < 7500.f &&
            proximity.facing_dot > 0.f &&
            Items::HasItem(state, MAGIC_WAND)
          ) {
            // @todo factor
            state.wand_sense_factor = Tachyon_Lerpf(state.wand_sense_factor, 1.f, 5.f * state.dt);

            if (state.wand_hold_factor > 0.5f) {
              fade_out += 0.5f * state.dt;

              if (fade_out > 1.f) fade_out = 1.f;
            }
          } else {
            fade_out -= 0.5f * state.dt;

            if (fade_out < 0.f) fade_out = 0.f;
          }
        }

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

          barrier.position.y -= entity.scale.y * 0.45f;
          barrier.scale.x *= 0.05f;
          barrier.scale.z *= 0.5f;
          barrier.color = barrier_color;

          barrier.position = tVec3f::lerp(
            barrier.position,
            barrier.position - tVec3f(0, 2.f * entity.scale.y, 0),
            fade_out
          );

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
          light.power = Tachyon_Lerpf(3.f, 0.f, Tachyon_EaseInOutf(fade_out));
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