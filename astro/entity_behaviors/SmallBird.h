#pragma once

#include "astro/entity_behaviors/behavior.h"
#include "astro/astrolabe.h"

namespace astro {
  behavior SmallBird {
    static bool IsFlyingAway(GameEntity& entity) {
      return entity.did_activate;
    }

    static void StartFlyingAway(GameEntity& entity, const float scene_time) {
      entity.game_activation_time = scene_time;
      entity.did_activate = true;

      // @todo refactor
      {
        float r = Tachyon_GetRandom();

        if (r < 0.33f) Sfx::PlaySound(SFX_BIRD_WINGS_1, 0.3f);
        else if (r < 0.66f) Sfx::PlaySound(SFX_BIRD_WINGS_2, 0.3f);
        else Sfx::PlaySound(SFX_BIRD_WINGS_3, 0.3f);
      }
    }

    static void HandleFlyingAway(GameEntity& entity, const State& state, const float scene_time) {
      tVec3f player_to_entity_xz = (entity.visible_position - state.player_position).xz().unit();
      float y_speed = 4000.f + 2000.f * (scene_time - entity.game_activation_time);

      entity.visible_position += player_to_entity_xz * 15000.f * state.dt;
      entity.visible_position.y += y_speed * state.dt;

      tVec3f away_direction = (player_to_entity_xz + tVec3f(0, 0.5f, 0)).unit();
      Quaternion away_rotation = Quaternion::FromDirection(away_direction, tVec3f(0, 1.f, 0));

      entity.visible_rotation = Quaternion::nlerp(entity.visible_rotation, away_rotation, 20.f * state.dt);
    }

    static void HandleIdleBehavior(Tachyon* tachyon, GameEntity& entity) {
      float scene_time = get_scene_time();

      // For birds we use enemy state fields for behavior,
      // but birds aren't actually enemies!
      auto& enemy = entity.enemy_state;

      // Randomly trigger a turn action whenever the bird's "mood" ""changes""
      {
        // Cycle between a few different durations to add variance to turn timings
        const float mood_durations[] = {
          0.8f,
          2.5f,
          2.f,
          5.4f,
          3.2f
        };

        int duration_cycle = int(enemy.last_mood_change_time) + int(entity.id);
        float mood_duration = mood_durations[duration_cycle % 5];

        if (time_since(enemy.last_mood_change_time) > mood_duration) {
          enemy.last_mood_change_time = get_scene_time();
        }
      }

      // Turning behavior; do a little jump and turn smoothly to a new angle
      {
        float time_since_turn = time_since(enemy.last_mood_change_time);

        if (enemy.last_mood_change_time != 0.f && time_since_turn < 0.2f) {
          float alpha = time_since_turn / 0.2f;

          entity.visible_position.y = entity.position.y + 250.f * sinf(alpha * t_PI);

          // Pick a "random" turn angle based on the last mood change time
          float angle = t_PI * sinf(2.123f * enemy.last_mood_change_time);
          Quaternion new_rotation = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), angle);

          entity.visible_rotation = Quaternion::nlerp(entity.visible_rotation, new_rotation, alpha);
        }
      }
    }

    static void Reset(GameEntity& entity) {
      entity.did_activate = false;
      entity.game_activation_time = -1.f;

      // Use a "random" timer offset for each bird
      entity.accumulation_value = 0.345f * float(entity.id);

      entity.visible_position = entity.position;
      entity.visible_rotation = entity.orientation;
    }

    addMeshes() {
      // Allow up to 500 placeholders
      meshes.small_bird_placeholder = MODEL_MESH("./astro/3d_models/small_bird/placeholder.obj", 500);

      // Allow up to 100 on-screen instances
      meshes.small_bird_body = MODEL_MESH("./astro/3d_models/small_bird/body.obj", 100);
      meshes.small_bird_head = MODEL_MESH("./astro/3d_models/small_bird/head.obj", 100);
      meshes.small_bird_wings = MODEL_MESH("./astro/3d_models/small_bird/wings.obj", 100);
      meshes.small_bird_left_wing = MODEL_MESH("./astro/3d_models/small_bird/left_wing.obj", 100);
      meshes.small_bird_right_wing = MODEL_MESH("./astro/3d_models/small_bird/right_wing.obj", 100);

      mesh(meshes.small_bird_placeholder).shadow_cascade_ceiling = 1;
      mesh(meshes.small_bird_body).shadow_cascade_ceiling = 1;
      mesh(meshes.small_bird_head).shadow_cascade_ceiling = 1;
      mesh(meshes.small_bird_wings).shadow_cascade_ceiling = 1;
      mesh(meshes.small_bird_left_wing).shadow_cascade_ceiling = 1;
      mesh(meshes.small_bird_right_wing).shadow_cascade_ceiling = 1;
    }

    getMeshes() {
      return_meshes({
        meshes.small_bird_body,
        meshes.small_bird_head,
        meshes.small_bird_wings,
        meshes.small_bird_left_wing,
        meshes.small_bird_right_wing,
      });
    }

    getPlaceholderMesh() {
      return meshes.small_bird_placeholder;
    }

    timeEvolve() {
      profile("  SmallBird::timeEvolve()");

      auto& meshes = state.meshes;

      const Quaternion head_rotations[] = {
        Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), 0),
        Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), 0.3f),
        Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), -0.3f),
        Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), 0.5f) * Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), -0.3f),
        Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), -0.5f) * Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), -0.3f)
      };

      reset_instances(meshes.small_bird_body);
      reset_instances(meshes.small_bird_head);
      reset_instances(meshes.small_bird_wings);
      reset_instances(meshes.small_bird_left_wing);
      reset_instances(meshes.small_bird_right_wing);

      for_entities(state.small_birds) {
        auto& entity = state.small_birds[i];

        if (!IsDuringActiveTime(entity, state)) {
          Reset(entity);

          continue;
        }

        if (!IsInRangeX(entity, state, 20000.f)) continue;
        if (!IsInRangeZ(entity, state, 20000.f)) continue;

        // Set visible scale once we're in range (so we can use SyncVisible())
        entity.visible_scale = entity.scale;

        // Timer for smaller movement actions
        entity.accumulation_value += state.dt;

        float player_distance = tVec3f::distance(state.player_position, entity.visible_position);

        if (IsFlyingAway(entity)) {
          HandleFlyingAway(entity, state, get_scene_time());
        } else if (player_distance < 6000.f && state.astro_turn_speed == 0.f) {
          StartFlyingAway(entity, get_scene_time());
        } else {
          HandleIdleBehavior(tachyon, entity);
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

          // If we haven't initialized the head object, or otherwise if the bird flies away,
          // sync it with the entity's visible attributes
          if (head.scale.x < entity.visible_scale.x || IsFlyingAway(entity)) {
            SyncVisible(head, entity);

          // Otherwise, randomly swivel the head
          } else {
            // @todo factor
            const float swivel_duration = 0.08f;

            int previous_rotation_index = int(entity.accumulation_value) % 5 - 1;
            if (previous_rotation_index < 0) previous_rotation_index = 4;

            int rotation_index = int(entity.accumulation_value) % 5;

            Quaternion old_rotation = entity.visible_rotation * head_rotations[previous_rotation_index];
            Quaternion new_rotation = entity.visible_rotation * head_rotations[rotation_index];

            float alpha = std::min(fmodf(entity.accumulation_value, 1.f), swivel_duration);
            alpha *= 1.f / swivel_duration;
            alpha = Tachyon_EaseOutQuad(alpha);

            head.rotation = Quaternion::nlerp(old_rotation, new_rotation, alpha);
            head.position = entity.visible_position;
          }

          head.color = tVec3f(0.2f);
          head.material = tVec4f(0.6f, 0, 0, 0.5f);

          commit(head);
        }

        // Wings (flying away)
        if (IsFlyingAway(entity)) {
          float flying_alpha = time_since(entity.game_activation_time) / 0.25f;
          float flap_rate = 35.f;
          float angle = sinf(flap_rate * get_scene_time());

          // Left
          {
            auto& left_wing = use_instance(meshes.small_bird_left_wing);

            SyncVisible(left_wing, entity);

            left_wing.rotation = left_wing.rotation * Quaternion::fromAxisAngle(tVec3f(0, 0, 1.f), -angle);
            left_wing.color = tVec3f(0.4f);
            left_wing.material = tVec4f(0.8f, 0, 0, 0.6f);

            commit(left_wing);
          }

          // Right
          {
            auto& right_wing = use_instance(meshes.small_bird_right_wing);

            SyncVisible(right_wing, entity);

            right_wing.rotation = right_wing.rotation * Quaternion::fromAxisAngle(tVec3f(0, 0, 1.f), angle);
            right_wing.color = tVec3f(0.4f);
            right_wing.material = tVec4f(0.8f, 0, 0, 0.6f);

            commit(right_wing);
          }

        // Wings (stationary)
        } else {
          auto& wings = use_instance(meshes.small_bird_wings);

          SyncVisible(wings, entity);

          wings.color = tVec3f(0.4f);
          wings.material = tVec4f(0.8f, 0, 0, 0.6f);

          commit(wings);
        }
      }
    }
  };
}