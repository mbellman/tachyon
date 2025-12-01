#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior Lamppost {
    addMeshes() {
      meshes.lamppost_placeholder = MODEL_MESH("./astro/3d_models/lamppost/placeholder.obj", 500);
      meshes.lamppost_stand = MODEL_MESH("./astro/3d_models/lamppost/stand.obj", 500);
      meshes.lamppost_frame = MODEL_MESH("./astro/3d_models/lamppost/frame.obj", 500);
      meshes.lamppost_lamp = MODEL_MESH("./astro/3d_models/lamppost/lamp.obj", 500);
    }

    getMeshes() {
      return_meshes({
        meshes.lamppost_stand,
        meshes.lamppost_frame,
        meshes.lamppost_lamp
      });
    }

    getPlaceholderMesh() {
      return meshes.lamppost_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      for_entities(state.lampposts) {
        auto& entity = state.lampposts[i];

        // Stand
        {
          auto& stand = objects(meshes.lamppost_stand)[i];

          Sync(stand, entity);

          commit(stand);
        }

        // Frame
        {
          auto& frame = objects(meshes.lamppost_frame)[i];

          Sync(frame, entity);

          frame.color = tVec3f(0.2f);
          frame.material = tVec4f(0.1f, 1.f, 0, 0);

          commit(frame);
        }

        // Lamp
        {
          auto& lamp = objects(meshes.lamppost_lamp)[i];

          Sync(lamp, entity);

          lamp.color = tVec4f(1.f, 0.8f, 0.6f, 1.f);

          commit(lamp);
        }

        // Light source
        {
          if (entity.light_id == -1) {
            entity.light_id = create_point_light();
          }

          auto& light = *get_point_light(entity.light_id);

          light.position = UnitEntityToWorldPosition(entity, tVec3f(-0.4f, 0.45f, 0));
          light.radius = 5000.f;
          light.color = tVec3f(1.f, 0.5f, 0.2f);
          light.power = 2.f;
          light.glow_power = 2.5f + sinf(3.f * get_scene_time());

          if (state.is_nighttime) {
            light.radius = 5000.f;
          } else {
            light.radius = 3000.f;
          }
        }

        // Collision
        entity.visible_position = entity.position;
        entity.visible_scale = entity.scale;
        entity.visible_rotation = entity.orientation;
      }
    }
  };
}