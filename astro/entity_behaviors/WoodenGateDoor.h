#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior WoodenGateDoor {
    addMeshes() {
      auto& meshes = state.meshes;

      meshes.wooden_gate_door_placeholder = MODEL_MESH("./astro/3d_models/wooden_gate_door/placeholder.obj", 500);
      meshes.wooden_gate_door = MODEL_MESH("./astro/3d_models/wooden_gate_door/door.obj", 500);
    }

    spawned() {
      auto& meshes = state.meshes;

      create(meshes.wooden_gate_door);
    }

    destroyed() {
      auto& meshes = state.meshes;

      RemoveLastObject(tachyon, meshes.wooden_gate_door);
    }

    createPlaceholder() {
      auto& meshes = state.meshes;

      return create(meshes.wooden_gate_door_placeholder);
    }

    destroyPlaceholders() {
      auto& meshes = state.meshes;

      remove_all(meshes.wooden_gate_door_placeholder);
    }

    timeEvolve() {
      auto& meshes = state.meshes;
      const float lifetime = 100.f;

      for_entities(state.wooden_gate_doors) {
        auto& entity = state.wooden_gate_doors[i];
        auto& door = objects(meshes.wooden_gate_door)[i];
        const float age = state.astro_time - entity.astro_start_time;

        door.position = entity.position;
        door.scale = entity.scale;
        door.rotation = entity.orientation;
        door.color = entity.tint;

        if (age < 0.f) door.scale = tVec3f(0.f);

        entity.visible_scale = door.scale;

        commit(door);
      }
    }
  };
}