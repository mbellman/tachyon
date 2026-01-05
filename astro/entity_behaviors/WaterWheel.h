#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior WaterWheel {
    static bool DidStopTurning(State& state, GameEntity& entity) {
      return entity.astro_end_time != 0.f && state.astro_time > entity.astro_end_time;
    }

    static float GetRotationVelocity(Tachyon* tachyon, State& state, GameEntity& entity) {
      if (DidStopTurning(state, entity)) {
        return 0.f;
      }

      if (
        entity.game_activation_time != -1.f &&
        state.astro_time >= entity.astro_activation_time
      ) {
        // Gradually stopping after activation time
        // @todo adjust stop duration to stop rotation at an interval of t_HALF_PI
        float stop_duration = 1.5f;
        float stop_alpha = time_since(entity.game_activation_time) / stop_duration;
        if (stop_alpha > 1.f) stop_alpha = 1.f;

        return Tachyon_Lerpf(0.7f, 0.f, stop_alpha) * state.dt;
      }

      float rotation_speed = 0.7f + 200.f * state.astro_turn_speed;

      return rotation_speed * state.dt;
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
          // Accumulate rotation
          entity.accumulation_value += GetRotationVelocity(tachyon, state, entity);

          // Wrap to keep the rotation value within a reasonable range,
          // and to avoid late-stage floating point precision errors
          entity.accumulation_value = fmodf(entity.accumulation_value, t_TAU);

          tVec3f rotation_axis = tVec3f(0, 0, 1.f);
          float rotation_angle = DidStopTurning(state, entity) ? 0.f : entity.accumulation_value;

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