#pragma once

#include "astro/entity_behaviors/behavior.h"
#include "astro/astrolabe.h"

namespace astro {
  behavior SmallBird {
    static bool StartedFlyingAway(GameEntity& entity) {
      return entity.did_activate;
    }

    static void FlyAway(GameEntity& entity,  const State& state) {
      tVec3f player_to_entity_xz = (entity.visible_position - state.player_position).xz().unit();

      entity.visible_position += player_to_entity_xz * 15000.f * state.dt;
      entity.visible_position.y += 4000.f * state.dt;

      Quaternion away_rotation = Quaternion::FromDirection(player_to_entity_xz, tVec3f(0, 1.f, 0));

      entity.visible_rotation = Quaternion::nlerp(entity.visible_rotation, away_rotation, 20.f * state.dt);
    }

    static void HandleIdleBehavior(Tachyon* tachyon, GameEntity& entity) {
      // For birds we use enemy state fields for behavior,
      // but birds aren't actually enemies!
      auto& enemy = entity.enemy_state;

      // Randomly trigger a turn action whenever the bird's "mood" ""changes""
      {
        if (time_since(enemy.last_mood_change_time) > 2.5f) {
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
          float angle = t_PI * sinf(3.456f * enemy.last_mood_change_time);
          Quaternion new_rotation = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), angle);

          entity.visible_rotation = Quaternion::nlerp(entity.visible_rotation, new_rotation, alpha);
        }
      }
    }

    static void StartFlyingAway(GameEntity& entity, const float scene_time) {
      entity.game_activation_time = scene_time;
      entity.did_activate = true;
    }

    static void Reset(GameEntity& entity) {
      entity.did_activate = false;
      entity.game_activation_time = -1.f;

      entity.accumulation_value = 0.f;

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

        if (StartedFlyingAway(entity)) {
          FlyAway(entity, state);
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
          if (head.scale.x < entity.visible_scale.x || StartedFlyingAway(entity)) {
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