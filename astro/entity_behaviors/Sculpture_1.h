#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior Sculpture_1 {
    static auto activation_sounds = {
      SFX_SCULPTURE_ACTIVATE_1,
      SFX_SCULPTURE_ACTIVATE_2,
      SFX_SCULPTURE_ACTIVATE_3,
      SFX_SCULPTURE_ACTIVATE_4
    };

    static void HandleWandAction(Tachyon* tachyon, State& state, GameEntity& entity) {
      entity.did_activate = true;
      entity.game_activation_time = get_scene_time();

      // Activation sound effect
      int sound_index = Tachyon_GetRandom(0, 3);
      Sound sound_effect = *(activation_sounds.begin() + sound_index);

      Sfx::PlaySound(sound_effect, 0.4f);

      // @todo factor
      {
        if (entity.requires_action) {
          entity.is_astro_synced = true;
        }

        // @todo check all sculpture chains
      }
    }

    static void Charge(Tachyon* tachyon, State& state, GameEntity& entity) {
      // Wait for the light be created first
      if (entity.light_id == -1) return;

      // Charge the light
      auto& light = *get_point_light(entity.light_id);

      light.power += 0.5f * state.dt;

      if (light.power >= 1.f) {
        light.power = 1.f;

        if (!entity.did_activate) {
          HandleWandAction(tachyon, state, entity);
        }
      }
    }

    addMeshes() {
      meshes.sculpture_1_placeholder = MODEL_MESH("./astro/3d_models/sculpture_1/placeholder.obj", 100);
      meshes.sculpture_1_stand = MODEL_MESH("./astro/3d_models/sculpture_1/stand.obj", 100);
      meshes.sculpture_1_wheel = MODEL_MESH("./astro/3d_models/sculpture_1/wheel.obj", 200);
    }

    getMeshes() {
      return_meshes({
        meshes.sculpture_1_stand,

        // Each sculpture instance has two spinning wheels
        meshes.sculpture_1_wheel,
        meshes.sculpture_1_wheel
      });
    }

    getPlaceholderMesh() {
      return meshes.sculpture_1_placeholder;
    }

    timeEvolve() {
      profile("  Sculpture_1::timeEvolve()");

      auto& meshes = state.meshes;

      tVec3f start_color = tVec3f(1.f, 1.f, 0.1f);
      tVec3f end_color = tVec3f(1.f, 0.5f, 0.2f);

      float scene_time = get_scene_time();
      float astro_rotation_speed = state.astro_turn_speed * 50.f * state.dt;

      reset_instances(meshes.sculpture_1_stand);
      reset_instances(meshes.sculpture_1_wheel);

      for_entities(state.sculpture_1s) {
        auto& entity = state.sculpture_1s[i];

        if (abs(state.player_position.x - entity.position.x) > 20000.f) continue;
        if (abs(state.player_position.z - entity.position.z) > 20000.f) continue;

        float life_progress = Tachyon_InverseLerp(entity.astro_start_time, entity.astro_end_time, state.astro_time);
        float decay_alpha = powf(life_progress, 3.f);
        tVec3f color = tVec3f::lerp(start_color, end_color, decay_alpha);

        float roughness = Tachyon_Lerpf(0.f, 1.f, decay_alpha);
        roughness *= roughness;

        // Interaction
        {
          auto proximity = GetEntityProximity(entity, state);

          if (
            proximity.distance < 6000.f &&
            proximity.facing_dot > 0.f &&
            IsDuringActiveTime(entity, state) &&
            Items::HasItem(state, MAGIC_WAND)
          ) {
            // @todo factor
            state.wand_sense_factor = Tachyon_Lerpf(state.wand_sense_factor, 1.f, 5.f * state.dt);

            if (state.wand_hold_factor > 0.5f) {
              Charge(tachyon, state, entity);
            }
          }
        }

        // Stand
        {
          auto& stand = use_instance(meshes.sculpture_1_stand);

          Sync(stand, entity);

          stand.color = color;
          stand.material = tVec4f(roughness, 1.f, 0, 0.4f);

          commit(stand);
        }

        // Wheels
        {
          const tVec3f rotation_axis = tVec3f(0, 0, 1.f);

          // Constant rotation
          entity.accumulation_value += state.dt + astro_rotation_speed;

          auto& wheel1 = use_instance(meshes.sculpture_1_wheel);
          auto& wheel2 = use_instance(meshes.sculpture_1_wheel);

          Sync(wheel1, entity);
          Sync(wheel2, entity);

          wheel1.position = UnitEntityToWorldPosition(entity, tVec3f(0, 1.5f, 0.15f));
          wheel2.position = UnitEntityToWorldPosition(entity, tVec3f(0, 1.5f, 0.2f));

          wheel1.scale = entity.scale * 0.5f;
          wheel1.rotation = entity.orientation * Quaternion::fromAxisAngle(rotation_axis, entity.accumulation_value);
          wheel1.color = color;
          wheel1.material = tVec4f(roughness, 1.f, 0, 0.4f);

          wheel2.scale = entity.scale * 0.3f;
          wheel2.rotation = entity.orientation * Quaternion::fromAxisAngle(rotation_axis, -entity.accumulation_value * 0.7f);
          wheel2.color = color;
          wheel2.material = tVec4f(roughness, 1.f, 0, 0.4f);

          // Disrepair
          float time_until_end = entity.astro_end_time - state.astro_time;
          if (time_until_end < 0.f) time_until_end = 0.f;

          // Larger wheel disrepair
          {
            if (time_until_end < 12.f) {
              wheel1.rotation = entity.orientation;
            }

            if (time_until_end == 0.f) {
              wheel1.position = UnitEntityToWorldPosition(entity, tVec3f(-0.4f, 0.05f, 0.3f));
              wheel1.rotation = Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), t_HALF_PI);
            }
          }

          // Smaller wheel disrepair
          {
            if (time_until_end < 20.f) {
              wheel2.rotation = entity.orientation;
            }

            if (time_until_end < 15.f) {
              wheel2.position = UnitEntityToWorldPosition(entity, tVec3f(0.5f, 0.05f, 0.2f));
              wheel2.rotation = Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), t_HALF_PI);
            }
          }

          commit(wheel1);
          commit(wheel2);
        }

        // Wand effects
        {
          const float speed_up_duration = 3.f;

          // Spinning faster immediately after interaction
          float spin_alpha = time_since(entity.game_activation_time) / speed_up_duration;
          if (spin_alpha < 0.f) spin_alpha = 0.f;
          if (spin_alpha > 1.f) spin_alpha = 1.f;

          if (spin_alpha < 0.2f) {
            spin_alpha = 5.f * spin_alpha;
          } else {
            spin_alpha = 1.f - (spin_alpha - 0.2f) / 0.8f;
          }

          entity.accumulation_value += state.dt * 3.f * spin_alpha;
        }

        // Light
        {
          const float minimum_power = 0.5f;
          tVec3f default_color = tVec3f(1.f, 0.4f, 0.1f);
          tVec3f activated_color = tVec3f(1.f, 0.6f, 0.3f);

          if (entity.light_id == -1) {
            // Initialization
            entity.light_id = create_point_light();

            get_point_light(entity.light_id)->power = minimum_power;
          }

          auto& light = *get_point_light(entity.light_id);

          // If we haven't fully activated the sculpture (e.g. while charging),
          // and if we've stopped charging it, let its power dwindle
          if (!entity.did_activate && state.wand_hold_factor < 0.5f) {
            light.power -= 0.5f * state.dt;

            if (light.power < minimum_power) light.power = minimum_power;
          }

          if (entity.did_activate) {
            light.power = 1.f;
          }

          // Kill the light if past end time
          if (state.astro_time > entity.astro_end_time) {
            light.power = 0.f;
          }

          light.position = UnitEntityToWorldPosition(entity, tVec3f(0, 1.5f, 0.25f));
          light.color = tVec3f::lerp(default_color, activated_color, light.power);
          light.radius = light.power * 2000.f;
          light.glow_power = 2.f;
        }

        // Collision
        entity.visible_position = entity.position;
        entity.visible_rotation = entity.orientation;
        entity.visible_scale = entity.scale;
      }
    }
  };
}