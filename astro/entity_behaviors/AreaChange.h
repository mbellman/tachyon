
#pragma once

#include "astro/entity_behaviors/behavior.h"
#include "astro/collision_system.h"

namespace astro {
  behavior AreaChange {
    addMeshes() {
      meshes.area_change_placeholder = CUBE_MESH(500);
    }

    getMeshes() {
      // Area changes have no in-game mesh
      return_meshes({});
    }

    getPlaceholderMesh() {
      return meshes.area_change_placeholder;
    }

    timeEvolve() {
      for (auto& entity : state.area_changes) {
        if (!IsInRangeX(entity, state, 15000.f)) continue;
        if (!IsInRangeZ(entity, state, 15000.f)) continue;

        auto plane = CollisionSystem::CreatePlane(entity.position, entity.scale, entity.orientation);

        if (CollisionSystem::IsPointOnPlane(state.player_position, plane)) {
          console_log("Area change!");

          // @todo
        }
      }
    }
  };
}
