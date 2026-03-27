#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior VantageSpot {
    addMeshes() {
      meshes.vantage_spot_placeholder = SPHERE_MESH(50);

      mesh(meshes.vantage_spot_placeholder).shadow_cascade_ceiling = 0;
    }

    getMeshes() {
      // Vantage spots don't have a specific in-game object
      return_meshes({});
    }

    getPlaceholderMesh() {
      return meshes.vantage_spot_placeholder;
    }

    timeEvolve() {
      bool close_to_any_vantage_spots = false;

      for_entities(state.vantage_spots) {
        auto& entity = state.vantage_spots[i];
        float player_distance = tVec3f::distance(state.player_position, entity.position);

        if (player_distance < 3000.f) {
          close_to_any_vantage_spots = true;

          break;
        }
      }

      if (close_to_any_vantage_spots) {
        if (!state.use_vantage_camera) {
          state.vantage_camera_start_time = get_scene_time();
        }

        state.use_vantage_camera = true;
      } else {
        state.use_vantage_camera = false;
      }
    }
  };
}