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
      profile("  Sculpture_1::timeEvolve()");

      auto& meshes = state.meshes;

      tVec3f start_color = tVec3f(1.f, 1.f, 0.1f);
      tVec3f end_color = tVec3f(1.f, 0.5f, 0.2f);

      float scene_time = get_scene_time();

      for_entities(state.sculpture_1s) {
        auto& entity = state.sculpture_1s[i];

        float life_progress = Tachyon_InverseLerp(entity.astro_start_time, entity.astro_end_time, state.astro_time);
        float alpha = powf(life_progress, 3.f);
        tVec3f color = tVec3f::lerp(start_color, end_color, alpha);
        float roughness = Tachyon_Lerpf(0.f, 1.f, alpha);

        float time_until_end = entity.astro_end_time - state.astro_time;
        if (time_until_end < 0.f) time_until_end = 0.f;

        // Stand
        {
          auto& stand = objects(meshes.sculpture_1_stand)[i];

          Sync(stand, entity);

          stand.color = color;
          stand.material = tVec4f(roughness, 1.f, 0, 0);

          commit(stand);
        }

        // Wheels
        {
          tVec3f rotation_axis = tVec3f(0, 0, 1.f);
          float rotation_angle = 1.f * (2.f * state.astro_time + get_scene_time());

          auto& wheel1 = objects(meshes.sculpture_1_wheel)[i * 2];
          auto& wheel2 = objects(meshes.sculpture_1_wheel)[i * 2 + 1];

          Sync(wheel1, entity);
          Sync(wheel2, entity);

          wheel1.position = UnitEntityToWorldPosition(entity, tVec3f(0, 1.5f, 0.15f));
          wheel2.position = UnitEntityToWorldPosition(entity, tVec3f(0, 1.5f, 0.2f));

          wheel1.scale = entity.scale * 0.5f;
          wheel1.rotation = entity.orientation * Quaternion::fromAxisAngle(rotation_axis, rotation_angle);
          wheel1.color = color;
          wheel1.material = tVec4f(roughness, 1.f, 0, 0);

          wheel2.scale = entity.scale * 0.3f;
          wheel2.rotation = entity.orientation * Quaternion::fromAxisAngle(rotation_axis, -rotation_angle * 0.7f);
          wheel2.color = color;
          wheel2.material = tVec4f(roughness, 1.f, 0, 0);

          // Larger wheel disrepair
          {
            if (time_until_end < 12.f) {
              wheel1.rotation = entity.orientation;
            }

            if (time_until_end == 0.f) {
              wheel1.position = UnitEntityToWorldPosition(entity, tVec3f(-0.4f, 0.05f, 0.3f));
              wheel1.rotation = Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), t_HALF_PI);
            }
          }

          // Smaller wheel disrepair
          {
            if (time_until_end < 20.f) {
              wheel2.rotation = entity.orientation;
            }

            if (time_until_end < 15.f) {
              wheel2.position = UnitEntityToWorldPosition(entity, tVec3f(0.5f, 0.05f, 0.2f));
              wheel2.rotation = Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), t_HALF_PI);
            }
          }

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