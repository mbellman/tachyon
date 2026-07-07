
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
        if (!IsInRangeX(entity, state, 10000.f)) continue;
        if (!IsInRangeZ(entity, state, 10000.f)) continue;

        // Floor tiles
        // @incomplete
        // @todo proper tile distribution/variation
        {
          int total_x_tiles = (int) ceilf(entity.scale.x / 2000.f);
          int total_z_tiles = (int) ceilf(entity.scale.z / 2000.f);

          float x_scale = entity.scale.x / float(total_x_tiles);
          float z_scale = entity.scale.z / float(total_z_tiles);

          for (int x = 0; x < total_x_tiles; x++) {
            for (int z = 0; z < total_z_tiles; z++) {
              auto& tile = use_instance(meshes.castle_tile);

              // @temporary
              tile.position.x = entity.position.x;
              tile.position.z = entity.position.z;

              tile.position.y = entity.position.y + entity.scale.y;

              tile.rotation = entity.orientation;

              tile.scale.x = x_scale;
              tile.scale.z = z_scale;
              tile.scale.y = 50.f;

              commit(tile);
            }
          }
        }
      }
    }
  };
}
