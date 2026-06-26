
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
          Location previous_location = state.current_location;

          // Match area change entity names to locations
          if (entity.unique_name == "tutorial") {
            state.current_location = Location::TUTORIAL;
          }
          else if (entity.unique_name == "divination_woodrealm") {
            state.current_location = Location::DIVINATION_WOODREALM;
          }
          else if (entity.unique_name == "riverway") {
            state.current_location = Location::DIVINATION_RIVERWAY;
          }
          else if (entity.unique_name == "promenade") {
            state.current_location = Location::DIVINATION_LAKE_PROMENADE;
          }
          else if (entity.unique_name == "lakefront_south") {
            state.current_location = Location::DIVINATION_LAKEFRONT_SOUTH;
          }

          if (state.current_location != previous_location) {
            state.last_area_change_time = get_scene_time();

            Sfx::PlaySound(SFX_AREA_CHANGE, 0.5f);
          }
        }
      }
    }
  };
}
