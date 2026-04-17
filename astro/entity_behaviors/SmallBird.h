#pragma once

#include "astro/entity_behaviors/behavior.h"
#include "astro/astrolabe.h"

namespace astro {
  behavior SmallBird {
    static void TurnRandomDirection(Tachyon* tachyon, GameEntity& entity) {
      // For birds we use enemy state fields for behavior,
      // but birds aren't actually enemies!
      auto& enemy = entity.enemy_state;

      if (time_since(enemy.last_mood_change_time) > 2.5f) {
        enemy.last_mood_change_time = get_scene_time();

        float angle = Tachyon_GetRandom(-t_PI, t_PI);

        entity.visible_rotation = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), angle);
      }
    }

    static void StartFlyingAway(GameEntity& entity, const float scene_time) {
      entity.game_activation_time = scene_time;
      entity.did_activate = true;
    }

    static bool DidFlyAway(GameEntity& entity) {
      return entity.game_activation_time != -1.f;
    }

    static void Reset(GameEntity& entity) {
      entity.did_activate = false;
      entity.game_activation_time = -1.f;

      entity.visible_position = entity.position;
      entity.visible_rotation = entity.orientation;
    }

    addMeshes() {
      meshes.small_bird_placeholder = MODEL_MESH("./astro/3d_models/small_bird/placeholder.obj", 500);
      meshes.small_bird_body = MODEL_MESH("./astro/3d_models/small_bird/body.obj", 500);
      meshes.small_bird_head = MODEL_MESH("./astro/3d_models/small_bird/head.obj", 500);
      meshes.small_bird_wings = MODEL_MESH("./astro/3d_models/small_bird/wings.obj", 500);

      mesh(meshes.small_bird_placeholder).shadow_cascade_ceiling = 1;
      mesh(meshes.small_bird_body).shadow_cascade_ceiling = 1;
      mesh(meshes.small_bird_head).shadow_cascade_ceiling = 1;
      mesh(meshes.small_bird_wings).shadow_cascade_ceiling = 1;
    }

    getMeshes() {
      return_meshes({
        meshes.small_bird_body,
        meshes.small_bird_head,
        meshes.small_bird_wings
      });
    }

    getPlaceholderMesh() {
      return meshes.small_bird_placeholder;
    }

    timeEvolve() {
      profile("  SmallBird::timeEvolve()");

      auto& meshes = state.meshes;

      reset_instances(meshes.small_bird_body);
      reset_instances(meshes.small_bird_head);
      reset_instances(meshes.small_bird_wings);

      for_entities(state.small_birds) {
        auto& entity = state.small_birds[i];

        // Outside of its astro time span, reset the bird to its original state
        if (
          state.astro_time < entity.astro_start_time ||
          state.astro_time > entity.astro_end_time
        ) {
          Reset(entity);

          continue;
        }

        if (!IsInRangeX(entity, state, 20000.f)) continue;
        if (!IsInRangeZ(entity, state, 20000.f)) continue;

        // Set visible scale once we're in range
        entity.visible_scale = entity.scale;

        float player_distance = tVec3f::distance(state.player_position, entity.visible_position);

        if (
          player_distance < 6000.f &&
          !DidFlyAway(entity) &&
          state.astro_turn_speed == 0.f
        ) {
          StartFlyingAway(entity, get_scene_time());
        } else {
          TurnRandomDirection(tachyon, entity);
        }

        if (DidFlyAway(entity)) {
          tVec3f player_to_entity_xz = (entity.visible_position - state.player_position).xz().unit();

          entity.visible_position += player_to_entity_xz * 15000.f * state.dt;
          entity.visible_position.y += 4000.f * state.dt;
        }

        // Body
        {
          auto& body = use_instance(meshes.small_bird_body);

          SyncVisible(body, entity);

          body.material = tVec4f(0.8f, 0, 0, 0.6f);

          commit(body);
        }

        // Head
        {
          auto& head = use_instance(meshes.small_bird_head);

          SyncVisible(head, entity);

          commit(head);
        }

        // Wings
        {
          auto& wings = use_instance(meshes.small_bird_wings);

          SyncVisible(wings, entity);

          wings.material = tVec4f(0.8f, 0, 0, 0.6f);

          commit(wings);
        }
      }
    }
  };
}