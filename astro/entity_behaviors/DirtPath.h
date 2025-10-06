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
        float remaining_time = entity.astro_end_time - state.astro_time;

        path.position = entity.position;
        path.position.y = -1450.f;
        path.scale = entity.scale;
        path.rotation = entity.orientation;
        path.color = entity.tint;

        // Reduce the size/conspicuousness of the path
        // as we approach its starting time
        if (age < 40.f && entity.astro_start_time != 0.f) {
          // @temporary
          const tVec3f ground_color = tVec3f(0.3f, 0.5f, 0.1f);
          float alpha = age / 40.f;

          path.color = tVec3f::lerp(ground_color, entity.tint, alpha);
          path.position.y = Tachyon_Lerpf(-1500.f, entity.position.y, alpha);

          if (entity.scale.x > entity.scale.z) {
            path.scale.z = entity.scale.z * alpha;
          } else {
            path.scale.x = entity.scale.x * alpha;
          }
        }

        // Erode the path toward its end time
        if (remaining_time < 40.f && entity.astro_end_time != 0.f) {
          // @temporary
          const tVec3f ground_color = tVec3f(0.3f, 0.5f, 0.1f);
          float alpha = remaining_time / 40.f;

          path.color = tVec3f::lerp(ground_color, entity.tint, alpha);
          path.position.y = Tachyon_Lerpf(-1500.f, entity.position.y, alpha);

          if (entity.scale.x > entity.scale.z) {
            path.scale.z = entity.scale.z * alpha;
          } else {
            path.scale.x = entity.scale.x * alpha;
          }
        }

        if (
          age < 0.f ||
          (remaining_time < 0.f && entity.astro_end_time != 0.f)
        ) {
          path.scale = tVec3f(0.f);
        }

        entity.visible_scale = path.scale;

        commit(path);
      }
    }
  };
}