#pragma once

#include "astro/entity_behaviors/behavior.h"
#include "astro/ui_system.h"

namespace astro {
  behavior Faerie {
    addMeshes() {
      meshes.faerie_placeholder = MODEL_MESH("./astro/3d_models/faerie/placeholder.obj", 500);
      meshes.faerie_left_wing = MODEL_MESH("./astro/3d_models/faerie/left_wing.obj", 500);
      meshes.faerie_right_wing = MODEL_MESH("./astro/3d_models/faerie/right_wing.obj", 500);
    }

    getMeshes() {
      return_meshes({
        meshes.faerie_left_wing,
        meshes.faerie_right_wing
      });
    }

    getPlaceholderMesh() {
      return meshes.faerie_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      for_entities(state.faeries) {
        auto& entity = state.faeries[i];

        // Left wing
        {
          auto& wing = objects(meshes.faerie_left_wing)[i];

          Sync(wing, entity);

          wing.color = tVec4f(0.2f, 0.4f, 0.8f, 0.1f);
          wing.material = tVec4f(0.2f, 0.5f, 0.1f, 1.f);

          commit(wing);
        }

        // Right wing
        {
          auto& wing = objects(meshes.faerie_right_wing)[i];

          Sync(wing, entity);

          wing.color = tVec4f(0.2f, 0.4f, 0.8f, 0.1f);
          wing.material = tVec4f(0.2f, 0.5f, 0.1f, 1.f);

          commit(wing);
        }

        // Light
        {
          if (entity.light_id == -1) {
            entity.light_id = create_point_light();
          }

          auto& light = *get_point_light(entity.light_id);

          light.position = UnitEntityToWorldPosition(entity, tVec3f(0, 0, 0.1f));
          light.radius = 1000.f;
          light.color = tVec3f(0.2f, 0.4f, 1.f);
        }

        // Interaction
        entity.visible_position = entity.position;
        entity.visible_scale = entity.scale;
        entity.visible_rotation = entity.orientation;
      }
    }
  };
}