#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior CastleWallFountain {
    addMeshes() {
      meshes.castle_wall_fountain_placeholder = MODEL_MESH("./astro/3d_models/castle_wall_fountain/placeholder.obj", 500);
      meshes.castle_wall_fountain = MODEL_MESH("./astro/3d_models/castle_wall_fountain/fountain.obj", 500);
      meshes.castle_wall_fountain_water = MODEL_MESH("./astro/3d_models/castle_wall_fountain/water.obj", 500);

      mesh(meshes.castle_wall_fountain).shadow_cascade_ceiling = 2;

      // The water stream obviously isn't foliage, but we can use foliage behavior
      // for the subtle warping/wobbling to mimic flow. We may want to rename the
      // mesh type to indicate its more general purpose.
      mesh(meshes.castle_wall_fountain_water).type = FOLIAGE_MESH;
    }

    getMeshes() {
      return_meshes({
        meshes.castle_wall_fountain,
        meshes.castle_wall_fountain_water
      });
    }

    getPlaceholderMesh() {
      return meshes.castle_wall_fountain_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      reset_instances(meshes.castle_wall_fountain);
      reset_instances(meshes.castle_wall_fountain_water);

      for (auto& entity : state.castle_wall_fountains) {
        if (!IsInRangeX(entity, state, 30000.f)) continue;
        if (!IsInRangeZ(entity, state, 40000.f)) continue;

        // Fountain
        {
          auto& fountain = use_instance(meshes.castle_wall_fountain);

          Sync(fountain, entity);

          fountain.material = tVec4f(0.6f, 0, 0, 1.f);

          commit(fountain);
        }

        // Water
        // @todo particles
        {
          auto& water = use_instance(meshes.castle_wall_fountain_water);

          Sync(water, entity);

          water.color = 0x6AF5;
          water.material = tVec4f(0.3f, 0, 0, 1.f);

          commit(water);
        }
      }
    }
  };
}
