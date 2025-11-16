#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior GlowFlower {
    addMeshes() {
      meshes.glow_flower_placeholder = MODEL_MESH("./astro/3d_models/glow_flower/placeholder.obj", 500);
      meshes.glow_flower_petals = MODEL_MESH("./astro/3d_models/glow_flower/petals.obj", 500);
    }

    getMeshes() {
      return_meshes({
        meshes.glow_flower_petals
      });
    }

    getPlaceholderMesh() {
      return meshes.glow_flower_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      const float lifetime = 50.f;
      const tVec4f default_material = tVec4f(0.6f, 0, 0, 0);
      const tVec4f activated_material = tVec4f(0.4f, 1.f, 0, 0.3f);
      const float activation_duration = 10.f;

      float time_since_casting_stun = time_since(state.spells.stun_start_time);

      // If we haven't cast stun yet, just pretend the full activation cycle has completed
      if (state.spells.stun_start_time == 0.f) time_since_casting_stun = activation_duration;

      auto& player_position = state.player_position;

      for_entities(state.glow_flowers) {
        auto& entity = state.glow_flowers[i];
        float life_progress = GetLivingEntityProgress(state, entity, lifetime);

        // Handling activated state
        float activation = 0.f;
        float deactivation = 0.f;
        float player_distance = tVec3f::distance(player_position, entity.visible_position);
        float t = time_since_casting_stun - player_distance * 0.000075f;

        if (t < activation_duration && (life_progress > 0.f && life_progress < 1.f)) {
          activation = t;
          if (activation < 0.f) activation = 0.f;
          if (activation > 1.f) activation = 1.f;

          if (t > activation_duration - 2.f) {
            float remaining_time = activation_duration - t;

            deactivation = remaining_time / 2.f;

            activation *= deactivation;
            activation *= activation;
            activation *= activation;
            activation *= activation;
          }
        }

        // Flower object and entity updates
        {
          auto& petals = objects(meshes.glow_flower_petals)[i];

          tVec3f flower_scale = entity.scale * sinf(life_progress * t_PI);
          tVec3f activated_scale_change = deactivation > 0.f
            ? tVec3f(150.f * activation)
            : tVec3f(150.f * Tachyon_EaseOutBackf(activation));

          float glow = deactivation > 0.f
            ? 0.4f * (activation * activation)
            : 0.4f * powf(activation, 0.1f);

          petals.position = entity.position;
          petals.rotation = entity.orientation;
          petals.scale = flower_scale + activated_scale_change;
          petals.color = tVec4f(entity.tint, glow);
          petals.material = tVec4f::lerp(default_material, activated_material, activation);

          entity.visible_scale = petals.scale;
          entity.visible_position = entity.position;

          commit(petals);
        }

        // Light updates
        {
          if (entity.light_id == -1) {
            // @todo handle disposal
            entity.light_id = create_point_light();
          }

          auto& light = *get_point_light(entity.light_id);
          float light_radius = 600.f * (entity.visible_scale.x / entity.scale.x);
          float activated_light_radius_change = 400.f * sqrtf(activation);

          light.position = entity.visible_position;
          light.position.y = entity.visible_position.y + entity.visible_scale.y * 0.25f;
          light.radius = light_radius + activated_light_radius_change;
          light.color = tVec3f(0.1f, 0.4f, 1.f);
          light.power = 2.f;
        }
      }
    }
  };
}