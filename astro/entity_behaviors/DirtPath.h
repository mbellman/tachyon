#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior DirtPath {
    addMeshes() {
      meshes.dirt_path_placeholder = PLANE_MESH(500);
      meshes.dirt_path = PLANE_MESH(500);
    }

    getMeshes() {
      return_meshes({
        meshes.dirt_path
      });
    }

    getPlaceholderMesh() {
      return meshes.dirt_path_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      for_entities(state.dirt_paths) {
        auto& entity = state.dirt_paths[i];
        auto& path = objects(meshes.dirt_path)[i];
        float age = state.astro_time - entity.astro_start_time;

        path.position = entity.position;
        path.position.y = -1450.f;
        path.scale = entity.scale;
        path.rotation = entity.orientation;
        path.color = entity.tint;

        if (age < 40.f) {
          // @temporary
          const tVec3f ground_color = tVec3f(0.3f, 0.5f, 0.1f);
          float alpha = age / 40.f;

          path.color = tVec3f::lerp(ground_color, entity.tint, alpha);
        }

        if (age < 0.f) path.scale = tVec3f(0.f);

        // Collision
        entity.visible_scale = path.scale;

        commit(path);
      }
    }
  };
}