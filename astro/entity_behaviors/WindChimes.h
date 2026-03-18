#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior WindChimes {
    addMeshes() {
      meshes.wind_chimes_placeholder = MODEL_MESH("./astro/3d_models/wind_chimes/placeholder.obj", 500);
      meshes.wind_chimes_stand = MODEL_MESH("./astro/3d_models/wind_chimes/stand.obj", 500);
      meshes.wind_chimes_pivot = MODEL_MESH("./astro/3d_models/wind_chimes/pivot.obj", 500);
      meshes.wind_chimes_hook = MODEL_MESH("./astro/3d_models/wind_chimes/hook.obj", 500);
    }

    getMeshes() {
      return_meshes({
        meshes.wind_chimes_stand,
        meshes.wind_chimes_pivot,
        meshes.wind_chimes_hook
      });
    }

    getPlaceholderMesh() {
      return meshes.wind_chimes_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      float scene_time = get_scene_time();

      for_entities(state.wind_chimes) {
        auto& entity = state.wind_chimes[i];

        // Collision
        entity.visible_position = entity.position;
        entity.visible_rotation = entity.orientation;
        entity.visible_scale = entity.scale;

        // Stand
        {
          auto& stand = objects(meshes.wind_chimes_stand)[i];

          Sync(stand, entity);

          stand.color = tVec3f(1.f, 0.9f, 0.4f);
          stand.material = tVec4f(0.4f, 1.f, 0, 0);

          commit(stand);
        }

        // Pivot
        {
          auto& pivot = objects(meshes.wind_chimes_pivot)[i];

          Sync(pivot, entity);

          pivot.position = UnitEntityToWorldPosition(entity, tVec3f(0, 0, 0.55f));
          pivot.color = tVec3f(1.f, 0.9f, 0.4f);
          pivot.material = tVec4f(0.4f, 1.f, 0, 0);

          pivot.rotation *= Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), 0.5f * sinf(scene_time));

          commit(pivot);
        }

        // Hook
        {
          auto& hook = objects(meshes.wind_chimes_hook)[i];

          Sync(hook, entity);

          hook.position = UnitEntityToWorldPosition(entity, tVec3f(0, 0, 0.55f));
          hook.color = tVec3f(1.f, 0.9f, 0.4f);
          hook.material = tVec4f(0.4f, 1.f, 0, 0);

          hook.rotation *= Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), -0.5f * sinf(scene_time + 0.5f));

          commit(hook);
        }
      }
    }
  };
}