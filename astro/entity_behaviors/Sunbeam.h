#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior Sunbeam {
    addMeshes() {
      meshes.sunbeam_placeholder = MODEL_MESH("./astro/3d_models/sunbeam.obj", 500);
      meshes.sunbeam = MODEL_MESH("./astro/3d_models/sunbeam.obj", 500);

      mesh(meshes.sunbeam_placeholder).shadow_cascade_ceiling = 0;
      mesh(meshes.sunbeam).shadow_cascade_ceiling = 0;

      mesh(meshes.sunbeam_placeholder).type = SUNBEAM_MESH;
      mesh(meshes.sunbeam).type = SUNBEAM_MESH;
    }

    getMeshes() {
      return_meshes({
        meshes.sunbeam
      });
    }

    getPlaceholderMesh() {
      return meshes.sunbeam_placeholder;
    }

    timeEvolve() {
      // profile("  Sunbeam::timeEvolve()");

      auto& scene = tachyon->scene;
      auto& meshes = state.meshes;

      tVec3f rotation_axis = tVec3f::cross(tVec3f(0, -1.f, 0), scene.primary_light_direction);
      float rotation_angle = acos(tVec3f::dot(tVec3f(0, -1.f, 0), scene.primary_light_direction));

      reset_instances(meshes.sunbeam);

      for_entities(state.sunbeams) {
        auto& entity = state.sunbeams[i];

        if (abs(state.player_position.x - entity.position.x) > 30000.f) continue;
        if (abs(state.player_position.z - entity.position.z) > 30000.f) continue;

        // @todo fade out inactive sunbeams
        if (!IsDuringActiveTime(entity, state)) continue;

        auto& sunbeam = use_instance(meshes.sunbeam);

        Sync(sunbeam, entity);

        sunbeam.rotation = Quaternion::fromAxisAngle(rotation_axis, rotation_angle);

        commit(sunbeam);
      }
    }
  }
}