#pragma once

#include "astro/entity_behaviors/behavior.h"
#include "astro/astrolabe.h"

namespace astro {
  behavior MoodLight {
    addMeshes() {
      meshes.mood_light_placeholder = MODEL_MESH("./astro/3d_models/mood_light/placeholder.obj", 500);

      mesh(meshes.mood_light_placeholder).shadow_cascade_ceiling = 0;
    }

    getMeshes() {
      return_meshes({});
    }

    getPlaceholderMesh() {
      return meshes.mood_light_placeholder;
    }

    timeEvolve() {
      for_entities(state.mood_lights) {
        auto& entity = state.mood_lights[i];

        // @todo check for editor in dev mode only
        if (!state.is_level_editor_open && !IsDuringActiveTime(entity, state)) {
          if (entity.light_id != -1) {
            remove_point_light(entity.light_id);

            entity.light_id = -1;
          }

          continue;
        }

        // Light source
        {
          if (entity.light_id == -1) {
            entity.light_id = create_point_light();
          }

          auto& light = *get_point_light(entity.light_id);

          light.position = entity.position;
          // @temporary
          // @todo make everything configurable
          light.radius = 4000.f;
          light.color = tVec3f(1.f, 0.5f, 0.1f);
          light.power = 3.f;
          light.glow_power = 0.f;
        }
      }
    }
  };
}