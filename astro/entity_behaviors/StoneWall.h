#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior StoneWall {
    addMeshes() {
      meshes.stone_wall_placeholder = MODEL_MESH("./astro/3d_models/stone_wall/placeholder.obj", 500);
      meshes.stone_wall = MODEL_MESH("./astro/3d_models/stone_wall/wall.obj", 500);
    }

    getMeshes() {
      return_meshes({
        meshes.stone_wall
      });
    }

    getPlaceholderMesh() {
      return meshes.stone_wall_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      for_entities(state.stone_walls) {
        auto& entity = state.stone_walls[i];
        auto& wall = objects(meshes.stone_wall)[i];

        bool is_active = (
          state.astro_time >= entity.astro_start_time &&
          state.astro_time <= entity.astro_end_time
        );

        wall.position = entity.position;
        wall.rotation = entity.orientation;
        wall.color = tVec3f(0.8f, 0.8f, 0.7f);

        if (is_active) {
          float age = state.astro_time - entity.astro_start_time;

          wall.scale = entity.scale;

          if (age < 10.f) wall.position.y = entity.position.y - 1000.f;
          if (age < 6.f) wall.position.y = entity.position.y - 2000.f;
          if (age < 3.f) wall.position.y = entity.position.y - 4000.f;
          if (age < 2.f) wall.position.y = entity.position.y - 5000.f;
          if (age < 1.f) wall.position.y = entity.position.y - 6000.f;
        } else {
          wall.scale = tVec3f(0.f);
        }

        entity.visible_scale = wall.scale;

        commit(wall);
      }
    }
  };
}