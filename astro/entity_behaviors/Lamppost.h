#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior Lamppost {
    static void TurnLampOn(Tachyon* tachyon, State& state, GameEntity& entity) {
      entity.did_activate = true;
      entity.astro_activation_time = state.astro_time;
      entity.game_activation_time = get_scene_time() + 0.25f;
    }

    static void TurnLampOff(Tachyon* tachyon, State& state, GameEntity& entity) {
      entity.did_activate = false;
      entity.astro_activation_time = state.astro_time;
      entity.game_activation_time = get_scene_time() + 0.25f;
    }

    static void HandleWandAction(Tachyon* tachyon, State& state, GameEntity& entity) {
      if (entity.did_activate) {
        TurnLampOff(tachyon, state, entity);
      } else {
        TurnLampOn(tachyon, state, entity);
      }
    }

    addMeshes() {
      meshes.lamppost_placeholder = MODEL_MESH("./astro/3d_models/lamppost/placeholder.obj", 500);
      meshes.lamppost_stand = MODEL_MESH("./astro/3d_models/lamppost/stand.obj", 500);
      meshes.lamppost_frame = MODEL_MESH("./astro/3d_models/lamppost/frame.obj", 500);
      meshes.lamppost_lamp = MODEL_MESH("./astro/3d_models/lamppost/lamp.obj", 500);

      mesh(meshes.lamppost_stand).shadow_cascade_ceiling = 2;
      mesh(meshes.lamppost_frame).shadow_cascade_ceiling = 2;
      mesh(meshes.lamppost_lamp).shadow_cascade_ceiling = 2;
    }

    getMeshes() {
      return_meshes({
        meshes.lamppost_stand,
        meshes.lamppost_frame,
        meshes.lamppost_lamp
      });
    }

    getPlaceholderMesh() {
      return meshes.lamppost_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      reset_instances(meshes.lamppost_stand);
      reset_instances(meshes.lamppost_frame);
      reset_instances(meshes.lamppost_lamp);

      for_entities(state.lampposts) {
        auto& entity = state.lampposts[i];

        if (!IsInRangeX(entity, state, 20000.f)) continue;
        if (!IsInRangeZ(entity, state, 20000.f)) continue;

        if (!IsDuringActiveTime(entity, state)) {
          if (entity.light_id != -1) {
            remove_point_light(entity.light_id);

            entity.light_id = -1;
          }

          continue;
        };

        bool is_light_active = entity.did_activate;

        float light_alpha = time_since(entity.game_activation_time) / 0.4f;
        if (light_alpha < 0.f) light_alpha = 0.f;
        if (light_alpha > 1.f) light_alpha = 1.f;
        light_alpha = Tachyon_EaseInOutf(light_alpha);

        // Stand
        {
          auto& stand = use_instance(meshes.lamppost_stand);

          Sync(stand, entity);

          stand.material = tVec4f(0.9f, 0, 0, 0.1f);

          commit(stand);
        }

        // Frame
        {
          auto& frame = use_instance(meshes.lamppost_frame);

          Sync(frame, entity);

          frame.color = tVec3f(0.2f);
          frame.material = tVec4f(0.1f, 1.f, 0, 0);

          commit(frame);
        }

        // Lamp
        {
          auto& lamp = use_instance(meshes.lamppost_lamp);

          Sync(lamp, entity);

          float emissivity = is_light_active
            ? Tachyon_Lerpf(0.5f, 1.f, light_alpha)
            : Tachyon_Lerpf(1.f, 0.5f, light_alpha);

          lamp.color = tVec4f(1.f, 0.8f, 0.6f, emissivity);

          commit(lamp);
        }

        // Light source
        {
          if (entity.light_id == -1) {
            entity.light_id = create_point_light();
          }

          auto& light = *get_point_light(entity.light_id);

          light.position = UnitEntityToWorldPosition(entity, tVec3f(-0.4f, 0.45f, 0));
          light.radius = 5000.f;
          light.color = tVec3f(1.f, 0.5f, 0.1f);

          float on_power = 4.f;
          float on_glow_power = 2.5f + sinf(3.f * get_scene_time());

          if (is_light_active) {
            if (light_alpha < 1.f) {
              // Fade light in
              light.power = Tachyon_Lerpf(0.f, on_power, light_alpha);
              light.glow_power = Tachyon_Lerpf(0.f, on_glow_power, light_alpha);
            } else {
              // Stay on
              light.power = on_power;
              light.glow_power = on_glow_power;
            }
          } else {
            if (light_alpha < 1.f) {
              // Fade light out
              light.power = Tachyon_Lerpf(on_power, 0.f, light_alpha);
              light.glow_power = Tachyon_Lerpf(on_glow_power, 0.f, light_alpha);
            } else {
              // Stay off
              light.power = 0.f;
              light.glow_power = 0.f;
            }
          }

          if (state.is_nighttime) {
            light.radius = 5000.f;
          } else {
            light.radius = 5000.f;
          }
        }

        // Collision
        entity.visible_position = entity.position;
        entity.visible_scale = entity.scale;
        entity.visible_rotation = entity.orientation;
      }
    }
  };
}