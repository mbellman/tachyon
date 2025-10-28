#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior House {
    addMeshes() {
      meshes.house_placeholder = MODEL_MESH("./astro/3d_models/house/placeholder.obj", 500);
      meshes.house_body = MODEL_MESH("./astro/3d_models/house/body.obj", 500);
      meshes.house_frame = MODEL_MESH("./astro/3d_models/house/frame.obj", 500);
      meshes.house_roof = MODEL_MESH("./astro/3d_models/house/roof.obj", 500);
      meshes.house_chimney = MODEL_MESH("./astro/3d_models/house/chimney.obj", 500);
    }

    getMeshes() {
      return_meshes({
        meshes.house_body,
        meshes.house_frame,
        meshes.house_roof,
        meshes.house_chimney
      });
    }

    getPlaceholderMesh() {
      return meshes.house_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;
      const float lifetime = 100.f;

      for_entities(state.houses) {
        auto& entity = state.houses[i];

        auto& body = objects(meshes.house_body)[i];
        auto& frame = objects(meshes.house_frame)[i];
        auto& roof = objects(meshes.house_roof)[i];
        auto& chimney = objects(meshes.house_chimney)[i];

        body.position =
        frame.position =
        roof.position =
        chimney.position = entity.position;

        body.scale =
        frame.scale =
        roof.scale =
        chimney.scale = entity.scale;

        body.rotation =
        frame.rotation =
        roof.rotation =
        chimney.rotation = entity.orientation;

        body.color = entity.tint;
        frame.color = tVec3f(0.6f, 0.4f, 0.3f);
        roof.color = tVec3f(0.5f, 0.4f, 0.3f);
        chimney.color = tVec3f(0.6f);

        commit(body);
        commit(frame);
        commit(roof);
        commit(chimney);
      }
    }
  };
}