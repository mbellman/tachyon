#pragma once

#include "astro/sfx.h"
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

      const tVec4f base_lamp_color = tVec4f(1.f, 0.8f, 0.4f, 0.f);
      const tVec4f illuminated_lamp_color = tVec4f(1.f, 0.8f, 0.5f, 0.4f);
      const float activation_delay = 0.5f;

      float time_since_casting_stun = tachyon->scene.scene_time - state.spells.stun_start_time;

      bool did_cast_stun = (
        time_since_casting_stun < activation_delay + 0.5f &&
        time_since_casting_stun > activation_delay
      );

      // @todo fade out after a duration if puzzle isn't solved
      for_entities(state.light_posts) {
        auto& entity = state.light_posts[i];
        float player_distance = tVec3f::distance(state.player_position, entity.position);

        bool is_illuminated = (
          entity.game_activation_time != -1.f &&
          state.astro_time >= entity.astro_activation_time
        );

        // Casting stun in proximity to the entity
        if (!is_illuminated && did_cast_stun && player_distance < 8000.f) {
          entity.game_activation_time = tachyon->scene.scene_time;
          entity.astro_activation_time = state.astro_time;

          Sfx::PlaySound(SFX_LIGHT_POST_ACTIVATE, 1.f);
        }

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
          lamp.material = tVec4f(1.f, 0, 0, 1.f);

          if (is_illuminated) {
            float alpha = 3.f * (tachyon->scene.scene_time - entity.game_activation_time);
            if (alpha < 0.f) alpha = 0.f;
            if (alpha > 1.f) alpha = 1.f;

            lamp.color = tVec4f::lerp(base_lamp_color, illuminated_lamp_color, alpha);
          } else {
            lamp.color = base_lamp_color;
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
          light.color = tVec3f(1.f, 0.9f, 0.7f);

          if (is_illuminated) {
            float alpha = tachyon->scene.scene_time - entity.game_activation_time;
            if (alpha < 0.f) alpha = 0.f;
            if (alpha > 1.f) alpha = 1.f;

            light.radius = 5000.f * alpha;
            light.power = 3.f * alpha;
            light.glow_power = (2.f + 0.5f * sinf(tachyon->scene.scene_time)) * alpha;
          } else {
            light.radius = 0.f;
            light.power = 0.f;
          }
        }
      }
    }
  };
}