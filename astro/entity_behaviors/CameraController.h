
#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior CameraController {
    addMeshes() {
      meshes.camera_controller_placeholder = MODEL_MESH("./astro/3d_models/camera_controller/placeholder.obj", 500);

      mesh(meshes.camera_controller_placeholder).shadow_cascade_ceiling = 0;
    }

    getMeshes() {
      return_meshes({});
    }

    getPlaceholderMesh() {
      return meshes.camera_controller_placeholder;
    }

    timeEvolve() {
      float closest_distance = FLT_MAX;
      std::string closest_camera_name = "";

      for (auto& entity : state.camera_controllers) {
        float dx = abs(state.player_position.x - entity.position.x);
        float dz = abs(state.player_position.z - entity.position.z);

        if (dx < 15000.f && dz < 15000.f) {
          float distance = tVec3f::distance(state.player_position.xz(), entity.position.xz());

          if (distance < closest_distance) {
            closest_distance = distance;
            closest_camera_name = entity.unique_name;
          }
        }
      }

      if (closest_camera_name != "") {
        auto& states = state.camera_state_map;

        if (states.find(closest_camera_name) != states.end()) {
          auto& camera_state = states.at(closest_camera_name);

          state.target_camera_height_adjustment = camera_state.height_adjustment;
        }
      }
    }
  };
}
