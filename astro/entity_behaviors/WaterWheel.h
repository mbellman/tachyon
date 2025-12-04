#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior WaterWheel {
    addMeshes() {
      meshes.water_wheel_placeholder = MODEL_MESH("./astro/3d_models/water_wheel/placeholder.obj", 500);
      meshes.water_wheel = MODEL_MESH("./astro/3d_models/water_wheel/wheel.obj", 500);

      mesh(meshes.water_wheel_placeholder).shadow_cascade_ceiling = 2;
      mesh(meshes.water_wheel).shadow_cascade_ceiling = 2;
    }

    getMeshes() {
      return_meshes({
        meshes.water_wheel
      });
    }

    getPlaceholderMesh() {
      return meshes.water_wheel_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      for_entities(state.water_wheels) {
        auto& entity = state.water_wheels[i];

        // Wheel
        {
          float rotation_speed =
            entity.astro_end_time != 0.f && state.astro_time > entity.astro_end_time
              ? 0.f
              : 0.5f;

          tVec3f rotation_axis = tVec3f(0, 0, 1.f);
          float rotation_angle = rotation_speed * (2.f * state.astro_time + get_scene_time());

          auto& wheel = objects(meshes.water_wheel)[i];

          Sync(wheel, entity);

          wheel.color = tVec3f(1.f, 0.6f, 0.3f);
          wheel.rotation = entity.orientation * Quaternion::fromAxisAngle(rotation_axis, rotation_angle);

          commit(wheel);
        }
      }
    }
  };
}