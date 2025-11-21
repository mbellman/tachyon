#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior Mushroom {
    addMeshes() {
      meshes.mushroom_placeholder = MODEL_MESH("./astro/3d_models/mushroom/placeholder.obj", 500);
      meshes.mushroom_body = MODEL_MESH("./astro/3d_models/mushroom/body.obj", 500);

      mesh(meshes.mushroom_placeholder).type = GRASS_MESH;
      mesh(meshes.mushroom_placeholder).shadow_cascade_ceiling = 2;

      mesh(meshes.mushroom_body).type = GRASS_MESH;
      mesh(meshes.mushroom_body).shadow_cascade_ceiling = 2;
    }

    getMeshes() {
      return_meshes({
        meshes.mushroom_body
      });
    }

    getPlaceholderMesh() {
      return meshes.mushroom_placeholder;
    }

    timeEvolve() {
      profile("  Mushroom::timeEvolve()");

      auto& meshes = state.meshes;

      const float lifetime = 50.f;

      // @temporary
      float emissivity = 0.2f;
      float light_power = 0.25f;

      for_entities(state.mushrooms) {
        auto& entity = state.mushrooms[i];
        float life_progress = GetLivingEntityProgress(state, entity, lifetime);

        // Mushroom
        {
          auto& body = objects(meshes.mushroom_body)[i];

          Sync(body, entity);

          body.color = tVec4f(entity.tint, emissivity);
          body.material = tVec4f(0.6f, 0, 0, 0.8f);

          commit(body);
        }

        entity.visible_rotation = entity.orientation;
        entity.visible_position = entity.position;
        entity.visible_scale = entity.scale;

        entity.tint = tVec3f(0.2f, 1.f, 0.4f);

        // Light updates
        {
          if (entity.light_id == -1) {
            // @todo handle disposal
            entity.light_id = create_point_light();
          }

          auto& light = *get_point_light(entity.light_id);

          light.position = UnitEntityToWorldPosition(entity, tVec3f(-0.025f, 0.15f, 0));
          light.radius = 1500.f;
          light.color = tVec3f(0.2f, 0.8f, 0.2f);
          light.power = 4.f + 1.f * sinf(get_scene_time() + entity.position.x);
          light.glow_power = 0.f;

          light.power = light_power;
        }
      }
    }
  };
}