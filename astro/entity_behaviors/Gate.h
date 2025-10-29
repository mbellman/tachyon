#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior Gate {
    addMeshes() {
      meshes.gate_placeholder = MODEL_MESH("./astro/3d_models/gate/placeholder.obj", 500);
      meshes.gate_body = MODEL_MESH("./astro/3d_models/gate/body.obj", 500);
    }

    getMeshes() {
      return_meshes({
        meshes.gate_body
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

        body.position = entity.position;
        body.scale = entity.scale;
        body.rotation = entity.orientation;
        body.color = entity.tint;

        commit(body);
      }
    }
  };
}