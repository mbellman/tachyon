#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior WaterWheel {
    static float GetRotationAngle(Tachyon* tachyon, State& state, GameEntity& entity) {
      if (entity.astro_end_time != 0.f && state.astro_time > entity.astro_end_time) {
        // Past end time; stopped
        return 0.f;
      }

      float t;

      if (entity.game_activation_time != -1.f) {
        // Gradually stopping after activation time
        float stop_alpha = time_since(entity.game_activation_time) / 1.5f;
        if (stop_alpha > 1.f) stop_alpha = 1.f;
        stop_alpha = Tachyon_EaseOutSine(stop_alpha * 0.667f);

        t = Tachyon_Lerpf(entity.game_activation_time, entity.game_activation_time + 1.5f, stop_alpha);
      } else {
        // Rotating normally
        t = get_scene_time();
      }

      return 0.5f * (2.f * state.astro_time + t);
    }

    addMeshes() {
      meshes.water_wheel_placeholder = MODEL_MESH("./astro/3d_models/water_wheel/placeholder.obj", 500);
      meshes.water_wheel = MODEL_MESH("./astro/3d_models/water_wheel/wheel.obj", 500);
      meshes.water_wheel_platform = MODEL_MESH("./astro/3d_models/water_wheel/platform.obj", 500);

      mesh(meshes.water_wheel_placeholder).shadow_cascade_ceiling = 2;
      mesh(meshes.water_wheel).shadow_cascade_ceiling = 2;
      mesh(meshes.water_wheel_platform).shadow_cascade_ceiling = 2;
    }

    getMeshes() {
      return_meshes({
        meshes.water_wheel,
        meshes.water_wheel_platform
      });
    }

    getPlaceholderMesh() {
      return meshes.water_wheel_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      float player_speed = state.player_velocity.magnitude();

      for_entities(state.water_wheels) {
        auto& entity = state.water_wheels[i];

        entity.visible_position = entity.position;
        entity.visible_scale = entity.scale;
        entity.visible_rotation = entity.orientation;

        // Wheel
        {
          // Rotation
          tVec3f rotation_axis = tVec3f(0, 0, 1.f);
          float rotation_angle = GetRotationAngle(tachyon, state, entity);

          auto& wheel = objects(meshes.water_wheel)[i];

          Sync(wheel, entity);

          wheel.color = tVec3f(1.f, 0.6f, 0.3f);
          wheel.rotation = entity.orientation * Quaternion::fromAxisAngle(rotation_axis, rotation_angle);

          commit(wheel);
        }

        // Platform
        {
          auto& platform = objects(meshes.water_wheel_platform)[i];

          Sync(platform, entity);

          platform.color = tVec3f(0.6f);
          platform.material = tVec4f(1.f, 0, 0, 0.2f);

          commit(platform);
        }

        // Proximity to control lever
        {
          tVec3f lever_position = UnitEntityToWorldPosition(entity, tVec3f(0, 0, 0.85f));
          float distance_from_lever = tVec3f::distance(state.player_position, lever_position);

          if (distance_from_lever < 2500.f && player_speed < 200.f) {
            UISystem::ShowTransientDialogue(tachyon, state, "[X] Operate Lever");

            if (
              !state.has_blocking_dialogue &&
              did_press_key(tKey::CONTROLLER_A)
            ) {
              // Reset player speed
              state.player_velocity = tVec3f(0.f);

              if (state.astro_time >= entity.astro_end_time) {
                UISystem::ShowBlockingDialogue(tachyon, state, "The mechanism resists.");
              } else {
                entity.astro_activation_time = state.astro_time;
                entity.game_activation_time = get_scene_time();

                UISystem::ShowBlockingDialogue(tachyon, state, "The wheel stopped.");
              }
            }
          }
        }
      }
    }
  };
}