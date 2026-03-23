#pragma once

#include "astro/entity_behaviors/behavior.h"
#include "astro/ui_system.h"

namespace astro {
  behavior Faerie {
    addMeshes() {
      meshes.faerie_placeholder = MODEL_MESH("./astro/3d_models/faerie/placeholder.obj", 500);
      meshes.faerie_left_wing = MODEL_MESH("./astro/3d_models/faerie/left_wing.obj", 500);
      meshes.faerie_right_wing = MODEL_MESH("./astro/3d_models/faerie/right_wing.obj", 500);

      mesh(meshes.faerie_left_wing).shadow_cascade_ceiling = 0;
      mesh(meshes.faerie_left_wing).type = FOLIAGE_MESH;
      mesh(meshes.faerie_right_wing).shadow_cascade_ceiling = 0;
      mesh(meshes.faerie_right_wing).type = FOLIAGE_MESH;
    }

    getMeshes() {
      return_meshes({
        meshes.faerie_left_wing,
        meshes.faerie_right_wing
      });
    }

    getPlaceholderMesh() {
      return meshes.faerie_placeholder;
    }

    handleEnemyBehavior() {
      auto& enemy = entity.enemy_state;

      // @todo slow down on approach
      enemy.speed = 750.f;
      enemy.mood = ENEMY_ENGAGED;

      tVec3f target_position = state.player_position + tVec3f(0, 1800.f, 0);
      tVec3f entity_to_target = target_position - entity.visible_position;
      float target_distance = entity_to_target.magnitude();

      if (target_distance > 10000.f) {
        return;
      }

      FaceTarget(entity, target_position, state.dt);

      if (target_distance > 2000.f) {
        tVec3f unit_entity_to_target = entity_to_target / target_distance;

        FollowPlayer(entity, unit_entity_to_target, state.dt);
      }
    }

    timeEvolve() {
      auto& meshes = state.meshes;
      float scene_time = get_scene_time();

      const float wing_speed = 1.3f;
      auto forward_rotation = Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), 0.35f);

      for_entities(state.faeries) {
        auto& entity = state.faeries[i];

        // The wing angle should be a function of speed * time, with a
        // per-entity offset controlled by its base position.
        float wing_angle = -0.2f + 0.9f * sinf(wing_speed * scene_time + entity.position.x);

        // Left wing
        {
          auto& wing = objects(meshes.faerie_left_wing)[i];

          Sync(wing, entity);

          wing.position = entity.visible_position;
          wing.rotation = entity.visible_rotation * forward_rotation * Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), wing_angle);
          wing.color = tVec4f(0.2f, 0.4f, 0.8f, 0.1f);
          wing.material = tVec4f(0.2f, 0.5f, 0.1f, 1.f);

          commit(wing);
        }

        // Right wing
        {
          auto& wing = objects(meshes.faerie_right_wing)[i];

          Sync(wing, entity);

          wing.position = entity.visible_position;
          wing.rotation = entity.visible_rotation * forward_rotation * Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), -wing_angle);
          wing.color = tVec4f(0.2f, 0.4f, 0.8f, 0.1f);
          wing.material = tVec4f(0.2f, 0.5f, 0.1f, 1.f);

          commit(wing);
        }

        // Light
        {
          if (entity.light_id == -1) {
            entity.light_id = create_point_light();
          }

          auto& light = *get_point_light(entity.light_id);

          light.position = UnitEntityToWorldPosition(entity, tVec3f(0, 0, 0.1f));
          light.radius = 1000.f;
          light.color = tVec3f(0.2f, 0.4f, 1.f);
          light.power = 2.f;
        }

        handle_enemy_behavior(Faerie);
      }
    }
  };
}