#pragma once

#include "astro/entity_behaviors/behavior.h"
#include "astro/time_evolution.h"

namespace astro {
  behavior WindChimes {
    static void Activate(Tachyon* tachyon, State& state, GameEntity& entity) {
      float scene_time = get_scene_time();

      entity.did_activate = true;
      entity.game_activation_time = scene_time;

      state.astro_particle_spawn_position = entity.position;
      state.last_wind_chimes_action_time = scene_time;
      state.last_used_wind_chimes_id = entity.id;

      // @temporary
      // @todo have different wind chimes for different time ranges
      if (entity.unique_name == "game_start_chimes") {
        if (state.astro_time == astro_time_periods.future) {
          // Future -> Present
          TimeEvolution::StartAstroTraveling(tachyon, state, astro_time_periods.present);
        } else {
          // Present -> Future
          TimeEvolution::StartAstroTraveling(tachyon, state, astro_time_periods.future);
        }

        return;
      }

      // @todo have different wind chimes for different time ranges
      if (state.target_astro_time == astro_time_periods.present) {
        // Present -> Past
        TimeEvolution::StartAstroTraveling(tachyon, state, astro_time_periods.past);
      } else {
        // Past -> Present
        TimeEvolution::StartAstroTraveling(tachyon, state, astro_time_periods.present);
      }
    }

    static void StartActivating(Tachyon* tachyon, State& state, GameEntity& entity) {
      entity.accumulation_value += state.dt;

      // @todo tune this + play a lead-in sound effect
      if (entity.accumulation_value > 0.5f) {
        Activate(tachyon, state, entity);
      }
    }

    static void HandleActivationBehavior(Tachyon* tachyon, State& state, GameEntity& entity) {
      // Allow us to start activating the wind chimes if:
      if (
        // We're not astro traveling
        state.astro_turn_speed == 0.f &&
        // We're not moving
        state.previous_move_delta == 0.f &&
        // The entity has not activated yet, or it has been long enough since last time
        (entity.game_activation_time == -1.f || time_since(entity.game_activation_time) > 5.f)
      ) {
        StartActivating(tachyon, state, entity);
      } else {
        // Not currently activating! Run the accumulation value back down.
        entity.accumulation_value -= state.dt;
        if (entity.accumulation_value < 0.f) entity.accumulation_value = 0.f;
      }
    }

    addMeshes() {
      meshes.wind_chimes_placeholder = MODEL_MESH("./astro/3d_models/wind_chimes/placeholder.obj", 500);
      meshes.wind_chimes_stand = MODEL_MESH("./astro/3d_models/wind_chimes/stand.obj", 500);
      meshes.wind_chimes_pivot = MODEL_MESH("./astro/3d_models/wind_chimes/pivot.obj", 500);
      meshes.wind_chimes_hook = MODEL_MESH("./astro/3d_models/wind_chimes/hook.obj", 500);
      meshes.wind_chimes_hook_2 = MODEL_MESH("./astro/3d_models/wind_chimes/hook_2.obj", 500);

      mesh(meshes.wind_chimes_stand).shadow_cascade_ceiling = 2;
      mesh(meshes.wind_chimes_pivot).shadow_cascade_ceiling = 2;
      mesh(meshes.wind_chimes_hook).shadow_cascade_ceiling = 2;
      mesh(meshes.wind_chimes_hook_2).shadow_cascade_ceiling = 2;
    }

    getMeshes() {
      return_meshes({
        meshes.wind_chimes_stand,
        meshes.wind_chimes_pivot,
        meshes.wind_chimes_hook,
        meshes.wind_chimes_hook_2
      });
    }

    getPlaceholderMesh() {
      return meshes.wind_chimes_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      float scene_time = get_scene_time();
      float player_speed = state.player_velocity.magnitude();

      for_entities(state.wind_chimes) {
        auto& entity = state.wind_chimes[i];

        // Collision
        entity.visible_position = entity.position;
        entity.visible_rotation = entity.orientation;
        entity.visible_scale = entity.scale;

        bool is_dismantled = entity.requires_action && !entity.is_astro_synced;

        // Interaction
        {
          auto proximity = GetEntityProximity(entity, state);

          if (
            proximity.distance < 5000.f &&
            proximity.facing_dot > 0.2f &&
            player_speed < 500.f
          ) {
            if (is_dismantled) {
              UISystem::ShowTransientDialogue(tachyon, state, "[X] Repair");

              if (
                player_speed < 200.f &&
                did_press_key(tKey::CONTROLLER_A)
              ) {
                if (Items::HasItem(state, CHIME_PARTS)) {
                  UISystem::ShowBlockingDialogue(tachyon, state, "Repaired the traveler's chime.");

                  // @todo rename did_take_action
                  entity.is_astro_synced = true;
                } else {
                  UISystem::ShowBlockingDialogue(tachyon, state, "Nothing handy to repair it with.");
                }
              }
            } else if (Items::HasItem(state, MAGIC_WAND)) {
              // @todo factor
              state.wand_sense_factor = Tachyon_Lerpf(state.wand_sense_factor, 1.f, 5.f * state.dt);

              if (state.wand_hold_factor > 0.5f) {
                HandleActivationBehavior(tachyon, state, entity);
              }
            }
          }
        }

        // Stand
        {
          auto& stand = objects(meshes.wind_chimes_stand)[i];

          Sync(stand, entity);

          stand.color = tVec3f(1.f, 0.7f, 0.2f);
          stand.material = tVec4f(0.2f, 1.f, 0, 0.2f);

          commit(stand);
        }

        // Pivot
        {
          auto& pivot = objects(meshes.wind_chimes_pivot)[i];

          Sync(pivot, entity);

          pivot.position = UnitEntityToWorldPosition(entity, tVec3f(0, 0, 0.55f));
          pivot.color = tVec3f(1.f, 0.7f, 0.2f);
          pivot.material = tVec4f(0.f, 1.f, 0, 0.5f);

          pivot.rotation *= Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), 0.5f * sinf(scene_time));

          if (is_dismantled) {
            pivot.rotation = entity.orientation;
          }

          commit(pivot);
        }

        // Hooks
        {
          auto& hook = objects(meshes.wind_chimes_hook)[i];
          auto& hook2 = objects(meshes.wind_chimes_hook_2)[i];

          Sync(hook, entity);
          Sync(hook2, entity);

          hook.position = UnitEntityToWorldPosition(entity, tVec3f(0, 0, 0.55f));
          hook.color = tVec3f(1.f, 0.7f, 0.2f);
          hook.material = tVec4f(0.f, 1.f, 0, 0.5f);

          hook2.position = UnitEntityToWorldPosition(entity, tVec3f(0, 0, 0.55f));
          hook2.color = tVec3f(1.f, 0.7f, 0.2f);
          hook2.material = tVec4f(0.f, 1.f, 0, 0.5f);

          hook.rotation *= Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), 0.5f * sinf(scene_time - 0.8f));
          hook2.rotation *= Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), 0.5f * sinf(scene_time - 1.6f));

          if (is_dismantled) {
            hook.scale = tVec3f(0.f);
            hook2.scale = tVec3f(0.f);
          }

          commit(hook);
          commit(hook2);
        }

        // Light
        {
          if (entity.light_id == -1) {
            entity.light_id = create_point_light();
          }

          auto& light = *get_point_light(entity.light_id);

          light.position = UnitEntityToWorldPosition(entity, tVec3f(0, 1.45f, 0.55f));
          light.radius = 2000.f;
          light.color = tVec3f(1.f, 0.6f, 0.3f);
          light.glow_power = 1.f;
          light.power = 1.4f + 0.5f * sinf(scene_time);

          // @temporary
          if (is_dismantled) {
            light.power = 0.f;
          }

          float time_since_activating = time_since(entity.game_activation_time);

          if (entity.game_activation_time > 0.f && time_since_activating < 4.f) {
            float alpha = sinf(t_PI * (time_since_activating / 4.f));

            light.radius += 3000.f * alpha;
            light.glow_power += alpha;
            light.color = tVec3f::lerp(light.color, tVec3f(1.f, 0.9f, 0.8f), alpha);
          }
        }
      }
    }
  };
}