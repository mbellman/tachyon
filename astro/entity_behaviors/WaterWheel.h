#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior WaterWheel {
    const static float BASE_ROTATION_RATE = 0.7f;
    const static float STOPPED_ROTATION = t_TAU;

    // Wrap rotation to between 0 and TAU
    static float GetNormalizedRotationAngle(float angle) {
      float new_angle = fmodf(angle, t_TAU);

      if (new_angle < 0.f) {
        new_angle += t_TAU;
      }

      return new_angle;
    }

    static bool DidStopTurning(State& state, GameEntity& entity) {
      return entity.astro_end_time != 0.f && state.astro_time > entity.astro_end_time;
    }

    static float GetRotationVelocity(Tachyon* tachyon, State& state, GameEntity& entity) {
      if (DidStopTurning(state, entity)) {
        return 0.f;
      }

      // @todo stay at 0 if we've already played the stopping animation to completion
      if (
        entity.game_activation_time != -1.f &&
        state.astro_time >= entity.astro_activation_time
      ) {
        // Slow down until we reach an interval of t_HALF_PI
        float current_rotation = entity.accumulation_value;
        float rotation_delta = STOPPED_ROTATION - current_rotation;
        float alpha = 1.f - rotation_delta / STOPPED_ROTATION;
        if (alpha > 1.f) alpha = 1.f;
        alpha *= alpha;
        alpha *= alpha;
        alpha *= alpha;

        return Tachyon_Lerpf(BASE_ROTATION_RATE, 0.f, alpha) * state.dt;
      }

      float rotation_speed = BASE_ROTATION_RATE + 200.f * state.astro_turn_speed;

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
          entity.accumulation_value = GetNormalizedRotationAngle(entity.accumulation_value);

          if (state.astro_time > entity.astro_activation_time) {
            entity.accumulation_value = 0.f;
          }

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

        // Player interactions
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

                if (entity.accumulation_value > t_PI + t_HALF_PI) {
                  // If we're almost rotated into the stopping position,
                  // set the rotation back halfway to give the stopping
                  // animation a little more time to proceed smoothly.
                  entity.accumulation_value -= t_PI;
                }

                UISystem::ShowBlockingDialogue(tachyon, state, "The wheel stopped turning.");
              }
            }
          }
        }

        // Event handling
        {
          if (
            !entity.did_activate &&
            entity.game_activation_time != -1.f &&
            time_since(entity.game_activation_time) > 4.f
          ) {
            GameEvents::StartEvent(tachyon, state, entity.unique_name);

            entity.did_activate = true;
          }
        }
      }
    }
  };
}