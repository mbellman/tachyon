#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior LightPost {
    addMeshes() {
      meshes.light_post_placeholder = MODEL_MESH("./astro/3d_models/light_post/placeholder.obj", 500);
      meshes.light_post_pillar = MODEL_MESH("./astro/3d_models/light_post/pillar.obj", 500);
      meshes.light_post_lamp = MODEL_MESH("./astro/3d_models/light_post/lamp.obj", 500);
    }

    getMeshes() {
      return_meshes({
        meshes.light_post_pillar,
        meshes.light_post_lamp
      });
    }

    getPlaceholderMesh() {
      return meshes.light_post_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      for_entities(state.light_posts) {
        auto& entity = state.light_posts[i];

        // @todo make conditional
        bool is_illuminated = true;

        // Pillar
        {
          auto& pillar = objects(meshes.light_post_pillar)[i];

          pillar.position = entity.position;
          pillar.scale = entity.scale;
          pillar.rotation = entity.orientation;
          pillar.color = tVec3f(0.7f, 0.6f, 0.6f);

          commit(pillar);
        }

        // Lamp
        {
          auto& lamp = objects(meshes.light_post_lamp)[i];

          lamp.position = entity.position;
          lamp.scale = entity.scale;
          lamp.rotation = entity.orientation;
          lamp.color = tVec3f(1.f, 0.8f, 0.4f);
          lamp.material = tVec4f(1.f, 0, 0, 1.f);

          if (is_illuminated) {
            lamp.color = tVec4f(1.f, 0.8f, 0.5f, 0.4f);
          }

          commit(lamp);
        }

        // Light
        {
          if (entity.light_id == -1) {
            // @todo handle disposal
            entity.light_id = create_point_light();
          }

          auto& light = *get_point_light(entity.light_id);

          light.position = entity.position;
          light.position.y += entity.scale.y * 1.3f;
          light.radius = 5000.f;
          light.power = 3.f;
          // light.glow_power =
          light.color = tVec3f(1.f, 0.9f, 0.7f);
        }
      }
    }
  };
}