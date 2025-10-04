#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior WoodenGateDoor {
    addMeshes() {
      meshes.wooden_gate_door_placeholder = MODEL_MESH("./astro/3d_models/wooden_gate_door/placeholder.obj", 500);
      meshes.wooden_gate_door = MODEL_MESH("./astro/3d_models/wooden_gate_door/door.obj", 500);
    }

    getMeshes() {
      return_meshes({
        meshes.wooden_gate_door
      });
    }

    getPlaceholderMesh() {
      return meshes.wooden_gate_door_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      for_entities(state.wooden_gate_doors) {
        auto& entity = state.wooden_gate_doors[i];
        auto& door = objects(meshes.wooden_gate_door)[i];

        bool is_active = (
          state.astro_time >= entity.astro_start_time &&
          state.astro_time <= entity.astro_end_time
        );

        door.position = entity.position;
        door.rotation = entity.orientation;
        door.color = entity.tint;

        if (is_active) {
          door.scale = entity.scale;
        } else {
          door.scale = tVec3f(0.f);
        }

        entity.visible_scale = door.scale;

        commit(door);
      }
    }
  };
}