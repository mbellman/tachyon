#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior Mushroom {
    addMeshes() {
      meshes.mushroom_placeholder = MODEL_MESH("./astro/3d_models/mushroom/placeholder.obj", 500);
      meshes.mushroom_body = MODEL_MESH("./astro/3d_models/mushroom/body.obj", 500);
      meshes.mushroom_spots = MODEL_MESH("./astro/3d_models/mushroom/spots.obj", 500);

      mesh(meshes.mushroom_placeholder).type = GRASS_MESH;
      mesh(meshes.mushroom_placeholder).shadow_cascade_ceiling = 2;

      mesh(meshes.mushroom_body).type = GRASS_MESH;
      mesh(meshes.mushroom_body).shadow_cascade_ceiling = 2;

      mesh(meshes.mushroom_spots).type = GRASS_MESH;
      mesh(meshes.mushroom_spots).shadow_cascade_ceiling = 1;
    }

    getMeshes() {
      return_meshes({
        meshes.mushroom_body,
        meshes.mushroom_spots
      });
    }

    getPlaceholderMesh() {
      return meshes.mushroom_placeholder;
    }

    timeEvolve() {
      profile("  Mushroom::timeEvolve()");

      auto& meshes = state.meshes;

      const float lifetime = 50.f;

      reset_instances(meshes.mushroom_body);
      reset_instances(meshes.mushroom_spots);

      // @todo growth
      for_entities(state.mushrooms) {
        auto& entity = state.mushrooms[i];

        if (abs(entity.position.x - state.player_position.x) > 20000.f) continue;
        if (abs(entity.position.z - state.player_position.z) > 20000.f) continue;

        float life_progress = GetLivingEntityProgress(state, entity, lifetime);

        // Mushroom
        {
          auto& body = use_instance(meshes.mushroom_body);

          Sync(body, entity);

          // @temporary
          if (state.is_nighttime) {
            body.color = tVec4f(entity.tint, 0.4f);
            body.material = tVec4f(0.6f, 0, 0, 1.f);
          } else {
            body.color = tVec4f(entity.tint, 0.2f);
            body.material = tVec4f(0.6f, 0, 0, 0.8f);
          }

          commit(body);
        }

        // Spots
        {
          auto& spots = use_instance(meshes.mushroom_spots);

          Sync(spots, entity);

          spots.color = tVec4f(1.f, 1.f, 1.f, 0.8f);
          spots.material = tVec4f(1.f, 0, 0, 1.f);

          commit(spots);
        }

        // @todo remove
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
          light.color = tVec3f(0.2f, 0.8f, 0.4f);
          light.glow_power = 0.f;

          // @temporary
          if (state.is_nighttime) {
            light.power = 4.f + sinf(4.f * get_scene_time() + entity.position.x * 0.01f);
          } else {
            light.power = 0.01f;
          }

          if (
            abs(state.player_position.x - entity.position.x) > 25000.f ||
            abs(state.player_position.z - entity.position.z) > 25000.f
          ) {
            light.power = 0.f;
          }
        }
      }
    }
  };
}