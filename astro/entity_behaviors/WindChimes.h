#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior WindChimes {
    addMeshes() {
      meshes.wind_chimes_placeholder = MODEL_MESH("./astro/3d_models/wind_chimes/placeholder.obj", 500);
      meshes.wind_chimes_stand = MODEL_MESH("./astro/3d_models/wind_chimes/stand.obj", 500);
      meshes.wind_chimes_pivot = MODEL_MESH("./astro/3d_models/wind_chimes/pivot.obj", 500);
      meshes.wind_chimes_hook = MODEL_MESH("./astro/3d_models/wind_chimes/hook.obj", 500);
      meshes.wind_chimes_hook_2 = MODEL_MESH("./astro/3d_models/wind_chimes/hook_2.obj", 500);

      mesh(meshes.wind_chimes_stand).shadow_cascade_ceiling = 2;
      mesh(meshes.wind_chimes_pivot).shadow_cascade_ceiling = 2;
      mesh(meshes.wind_chimes_hook).shadow_cascade_ceiling = 2;
      mesh(meshes.wind_chimes_hook_2).shadow_cascade_ceiling = 2;
    }

    getMeshes() {
      return_meshes({
        meshes.wind_chimes_stand,
        meshes.wind_chimes_pivot,
        meshes.wind_chimes_hook,
        meshes.wind_chimes_hook_2
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

          stand.color = tVec3f(1.f, 0.7f, 0.2f);
          stand.material = tVec4f(0.4f, 1.f, 0, 0.2f);

          commit(stand);
        }

        // Pivot
        {
          auto& pivot = objects(meshes.wind_chimes_pivot)[i];

          Sync(pivot, entity);

          pivot.position = UnitEntityToWorldPosition(entity, tVec3f(0, 0, 0.55f));
          pivot.color = tVec4f(1.f, 0.7f, 0.2f, 0.1f);
          pivot.material = tVec4f(0.4f, 1.f, 0, 1.f);

          pivot.rotation *= Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), 0.5f * sinf(scene_time));

          commit(pivot);
        }

        // Hooks
        {
          auto& hook = objects(meshes.wind_chimes_hook)[i];
          auto& hook2 = objects(meshes.wind_chimes_hook_2)[i];

          Sync(hook, entity);
          Sync(hook2, entity);

          hook.position = UnitEntityToWorldPosition(entity, tVec3f(0, 0, 0.55f));
          hook.color = tVec4f(1.f, 0.7f, 0.2f, 0.1f);
          hook.material = tVec4f(0.4f, 1.f, 0, 1.f);

          hook2.position = UnitEntityToWorldPosition(entity, tVec3f(0, 0, 0.55f));
          hook2.color = tVec4f(1.f, 0.7f, 0.2f, 0.1f);
          hook2.material = tVec4f(0.4f, 1.f, 0, 1.f);

          hook.rotation *= Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), 0.5f * sinf(scene_time - 0.8f));
          hook2.rotation *= Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), 0.5f * sinf(scene_time - 1.6f));

          commit(hook);
          commit(hook2);
        }
      }
    }
  };
}