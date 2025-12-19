#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior Sculpture_1 {
    addMeshes() {
      meshes.sculpture_1_placeholder = MODEL_MESH("./astro/3d_models/sculpture_1/placeholder.obj", 100);
      meshes.sculpture_1_stand = MODEL_MESH("./astro/3d_models/sculpture_1/stand.obj", 100);
      meshes.sculpture_1_wheel = MODEL_MESH("./astro/3d_models/sculpture_1/wheel.obj", 200);
    }

    getMeshes() {
      return_meshes({
        meshes.sculpture_1_stand,

        // Each sculpture instance has two spinning wheels
        meshes.sculpture_1_wheel,
        meshes.sculpture_1_wheel
      });
    }

    getPlaceholderMesh() {
      return meshes.sculpture_1_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      float scene_time = get_scene_time();

      for_entities(state.sculpture_1s) {
        auto& entity = state.sculpture_1s[i];

        // Stand
        {
          auto& stand = objects(meshes.sculpture_1_stand)[i];

          Sync(stand, entity);

          stand.color = tVec3f(1.f, 0.6f, 0.2f);
          stand.material = tVec4f(0.4f, 1.f, 0, 0);

          commit(stand);
        }

        // Wheels
        {
          auto& wheel1 = objects(meshes.sculpture_1_wheel)[i * 2];
          auto& wheel2 = objects(meshes.sculpture_1_wheel)[i * 2 + 1];

          Sync(wheel1, entity);
          Sync(wheel2, entity);

          wheel1.position = UnitEntityToWorldPosition(entity, tVec3f(0, 1.5f, 0.15f));
          wheel2.position = UnitEntityToWorldPosition(entity, tVec3f(0, 1.5f, 0.2f));

          wheel1.scale = entity.scale * 0.5f;
          wheel1.rotation = entity.orientation * Quaternion::fromAxisAngle(tVec3f(0, 0, 1.f), scene_time);
          wheel1.color = tVec3f(1.f, 0.5f, 0.2f);
          wheel1.material = tVec4f(0.2f, 1.f, 0, 0);

          wheel2.scale = entity.scale * 0.3f;
          wheel2.rotation = entity.orientation * Quaternion::fromAxisAngle(tVec3f(0, 0, 1.f), -scene_time * 0.7f);
          wheel2.color = tVec3f(1.f, 0.5f, 0.2f);
          wheel2.material = tVec4f(0.5f, 1.f, 0, 0);

          commit(wheel1);
          commit(wheel2);
        }

        // Collision
        entity.visible_position = entity.position;
        entity.visible_rotation = entity.orientation;
        entity.visible_scale = entity.scale;
      }
    }
  };
}