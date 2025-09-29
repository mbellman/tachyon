#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior RiverLog {
    addMeshes() {
      meshes.river_log_placeholder = MODEL_MESH("./astro/3d_models/river_log/log.obj", 500);
      meshes.river_log = MODEL_MESH("./astro/3d_models/river_log/log.obj", 500);
    }

    getMeshes() {
      return_meshes({
        meshes.river_log
      });
    }

    getPlaceholderMesh() {
      return meshes.river_log_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      // @todo @optimize only iterate over on-screen/in-range entities
      // once that list is built
      for_entities(state.river_logs) {
        auto& entity = state.river_logs[i];
        auto& trunk = objects(meshes.river_log)[i];
        float surface_oscillation = sinf(tachyon->running_time * 0.6f + entity.position.x);
        float swivel_oscillation = sinf(tachyon->running_time + entity.position.x);

        float surface_height = -trunk.scale.y * 0.15f + trunk.scale.y * 0.04f * sinf(surface_oscillation);
        Quaternion swivel = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), 0.025f * swivel_oscillation);

        trunk.position = entity.position;
        trunk.position.y = state.water_level + surface_height;
        trunk.rotation = entity.orientation * swivel;
        trunk.color = entity.tint;
        trunk.scale = entity.scale;

        // Collision
        entity.visible_scale = trunk.scale;

        commit(trunk);
      }
    }
  };
}