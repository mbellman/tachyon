#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior Lamp {
    addMeshes() {
      meshes.lamp_placeholder = MODEL_MESH("./astro/3d_models/lamp/placeholder.obj", 500);
      meshes.lamp_frame = MODEL_MESH("./astro/3d_models/lamp/frame.obj", 500);
      meshes.lamp_light = MODEL_MESH("./astro/3d_models/lamp/light.obj", 500);

      mesh(meshes.lamp_placeholder).shadow_cascade_ceiling = 1;
      mesh(meshes.lamp_frame).shadow_cascade_ceiling = 2;
      mesh(meshes.lamp_light).shadow_cascade_ceiling = 2;
    }

    getMeshes() {
      return_meshes({
        meshes.lamp_frame,
        meshes.lamp_light
      });
    }

    getPlaceholderMesh() {
      return meshes.lamp_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      reset_instances(meshes.lamp_frame);
      reset_instances(meshes.lamp_light);

      for_entities(state.lamps) {
        auto& entity = state.lamps[i];

        if (!IsDuringActiveTime(entity, state)) {
          // Hide light when it isn't the entity's active time
          if (state.astro_turn_speed != 0.f && entity.light_id != -1) {
            auto& light = *get_point_light(entity.light_id);

            light.power = 0.f;
          }

          continue;
        }

        if (!state.use_vantage_camera) {
          if (abs(state.player_position.x - entity.position.x) > 15000.f) continue;
          if (abs(state.player_position.z - entity.position.z) > 15000.f) continue;
        }

        // Frame
        {
          auto& frame = use_instance(meshes.lamp_frame);

          Sync(frame, entity);

          frame.color = tVec3f(0.2f);
          frame.material = tVec4f(0.1f, 1.f, 0, 0);

          commit(frame);
        }

        // Lamp
        {
          auto& lamp = use_instance(meshes.lamp_light);

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
          float oscillation_alpha = entity.position.x + 2.f * get_scene_time();

          light.position = entity.position;
          light.radius = 2000.f;
          light.color = tVec3f(1.f, 0.3f, 0.1f);

          light.power = 3.f + 0.5f * sinf(oscillation_alpha);
          light.glow_power = 2.5f + sinf(oscillation_alpha);
        }
      }
    }
  };
}