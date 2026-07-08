
#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior CastleTower {
    addMeshes() {
      meshes.castle_tower_placeholder = MODEL_MESH("./astro/3d_models/castle_tower/placeholder.obj", 500);
      meshes.castle_tower = MODEL_MESH("./astro/3d_models/castle_tower/tower.obj", 500);
    }

    getMeshes() {
      return_meshes({
        meshes.castle_tower
      });
    }

    getPlaceholderMesh() {
      return meshes.castle_tower_placeholder;
    }

    timeEvolve() {
      profile("  CastleTower::timeEvolve()");

      auto& meshes = state.meshes;

      reset_instances(meshes.castle_tower);
      reset_instances(meshes.castle_tile);

      for_entities(state.castle_towers) {
        auto& entity = state.castle_towers[i];

        // Wall structure
        {
          auto& tower = use_instance(meshes.castle_tower);

          Sync(tower, entity);

          tower.material = tVec4f(0.6f, 0, 0, 1.f);

          commit(tower);
        }

        // Skip additional visual features if the structure is far enough away
        if (!IsInRangeX(entity, state, 20000.f)) continue;
        if (!IsInRangeZ(entity, state, 20000.f)) continue;

        // Floor tiles
        // @todo color/material variation
        {
          const float ratio = 0.6f;

          float wall_x_width = entity.scale.x * 2.f * ratio;
          float wall_z_width = entity.scale.z * 2.f * ratio;

          int total_x_tiles = (int) ceilf(wall_x_width / 3000.f);
          int total_z_tiles = (int) ceilf(wall_z_width / 3000.f);

          float x_scale = wall_x_width / float(total_x_tiles);
          float z_scale = wall_z_width / float(total_z_tiles);

          tVec3f x_axis = entity.orientation.toMatrix4f() * tVec3f(1.f, 0, 0);
          tVec3f z_axis = entity.orientation.toMatrix4f() * tVec3f(0, 0, 1.f);

          tVec3f top_left = entity.position;
          top_left.y += entity.scale.y;
          top_left -= x_axis * (x_scale * (total_x_tiles - 1) * 0.5f);
          top_left -= z_axis * (z_scale * (total_z_tiles - 1) * 0.5f);

          for (int z = 0; z < total_z_tiles; z++) {
            tVec3f start = top_left + z_axis * z_scale * z;

            for (int x = 0; x < total_x_tiles; x++) {
              if (count_used_instances(meshes.castle_tile) == 150) {
                console_warn("Too many castle tiles!");

                return;
              }

              auto& tile = use_instance(meshes.castle_tile);

              tile.position = start + x_axis * x_scale * x;

              tile.rotation = entity.orientation;

              tile.scale.x = 0.832f * ratio * x_scale;
              tile.scale.z = 0.832f * ratio * z_scale;
              tile.scale.y = 50.f;

              tile.material = tVec4f(0.8f, 0, 0, 0.7f);

              commit(tile);
            }
          }
        }
      }
    }
  };
}
