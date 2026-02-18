#pragma once

#include "engine/tachyon.h"
#include "astro/entities.h"
#include "astro/game_events.h"
#include "astro/game_state.h"
#include "astro/player_character.h"
#include "astro/sfx.h"
#include "astro/simple_animation.h"
#include "astro/ui_system.h"

// @todo move to engine
#define CUBE_MESH(total) Tachyon_AddMesh(tachyon, Tachyon_CreateCubeMesh(), total)
#define SPHERE_MESH(total) Tachyon_AddMesh(tachyon, Tachyon_CreateSphereMesh(20), total)
#define PLANE_MESH(total) Tachyon_AddMesh(tachyon, Tachyon_CreatePlaneMesh(), total)
#define MODEL_MESH(path, total) Tachyon_AddMesh(tachyon, Tachyon_LoadMesh(path), total)
#define MODEL_MESH_LOD_2(lod_1_path, lod_2_path, total) Tachyon_AddMesh(tachyon, Tachyon_LoadMesh(lod_1_path), Tachyon_LoadMesh(lod_2_path), total)

#define behavior namespace

#define return_meshes(...)\
  static std::vector<uint16> __mesh_list = __VA_ARGS__;\
  return __mesh_list;\

#define addMeshes() static void _AddMeshes(Tachyon* tachyon, MeshIds& meshes)
#define getMeshes() static const std::vector<uint16>& _GetMeshes(const MeshIds& meshes)
#define getPlaceholderMesh() static uint16 _GetPlaceholderMesh(const MeshIds& meshes)
#define timeEvolve() static void _TimeEvolve(Tachyon* tachyon, State& state)
#define handleEnemyBehavior() static void _HandleEnemyBehavior(Tachyon* tachyon, State& state, GameEntity& entity)

#define handle_enemy_behavior(__behavior) __behavior::_HandleEnemyBehavior(tachyon, state, entity)

namespace astro {
  static float GetLivingEntityProgress(State& state, const GameEntity& entity, const float lifetime) {
    float entity_age = state.astro_time - entity.astro_start_time;
    if (entity_age < 0.f) return 0.f;
    if (entity_age > lifetime) return 1.f;

    return entity_age / lifetime;
  }

  static void Sync(tObject& object, GameEntity& entity) {
    object.position = entity.position;
    object.rotation = entity.orientation;
    object.scale = entity.scale;
    object.color = entity.tint;
  }

  static tVec3f UnitEntityToWorldPosition(const GameEntity& entity, const tVec3f& position) {
    tVec3f translation = entity.visible_position;
    Quaternion rotation = entity.visible_rotation;
    tVec3f scale = entity.visible_scale;

    return translation + rotation.toMatrix4f() * (position * scale);
  }

  static tVec3f UnitObjectToWorldPosition(const tObject& object, const tVec3f& position) {
    tVec3f translation = object.position;
    Quaternion rotation = object.rotation;
    tVec3f scale = object.scale;

    return translation + rotation.toMatrix4f() * (position * scale);
  }

  static void Jitter(GameEntity& entity, const float amount) {
    entity.visible_position.x += Tachyon_GetRandom(-amount, amount);
    entity.visible_position.z += Tachyon_GetRandom(-amount, amount);

    float angle = Tachyon_GetRandom(-0.3f, 0.3f);

    entity.visible_rotation = entity.orientation * Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), angle);
  }

  // @todo allow a tweening factor
  static void FacePlayer(GameEntity& entity, State& state) {
    tVec3f entity_to_player = state.player_position - entity.visible_position;
    Quaternion facing_direction = Quaternion::FromDirection(entity_to_player.unit(), tVec3f(0, 1.f, 0));

    // @todo use nlerp
    entity.visible_rotation = Quaternion::slerp(entity.visible_rotation, facing_direction, 2.f * state.dt);
  }

  static void FollowPlayer(GameEntity& entity, const tVec3f& player_direction, const float dt) {
    entity.visible_position += player_direction * entity.enemy_state.speed * dt;
  }

  static void AvoidPlayer(GameEntity& entity, const tVec3f& player_direction, const float dt) {
    entity.visible_position -= player_direction * entity.enemy_state.speed * dt;
  }

  static void PreventEntityCollisions(GameEntity& entity, const std::vector<GameEntity>& entities, const float radius_factor) {
    for (auto& other : entities) {
      if (IsSameEntity(entity, other)) {
        continue;
      }

      tVec3f entity_to_entity = entity.visible_position.xz() - other.visible_position.xz();
      float distance = entity_to_entity.magnitude();
      float minimum_distance = radius_factor * (entity.visible_scale.x + other.visible_scale.x);

      if (distance < minimum_distance) {
        tVec3f unit_direction = entity_to_entity / distance;

        entity.visible_position = other.visible_position + unit_direction * minimum_distance;
      }
    }
  }

  static void PreventEntityPlayerCollision(GameEntity& entity, const tVec3f& player_position, const tVec3f& player_direction, const float player_distance, const float radius_factor) {
    float minimum_distance = radius_factor * entity.visible_scale.x + 500.f;

    if (player_distance < minimum_distance) {
      entity.visible_position = player_position + player_direction.invert() * minimum_distance;
    }
  }

  static void TrackRecentPositions(GameEntity& entity, const float scene_time) {
    float time_since_last_recent_position = scene_time - entity.last_recent_position_record_time;

    if (time_since_last_recent_position > 1.f && entity.enemy_state.mood != ENEMY_IDLE) {
      auto& recent_positions = entity.recent_positions;

      if (recent_positions.size() > 30) {
        recent_positions.erase(recent_positions.begin());
      }

      recent_positions.push_back(entity.visible_position);

      entity.last_recent_position_record_time = scene_time;
    }
  }

  static void ReloadRecentPosition(GameEntity& entity, const float scene_time) {
    float time_since_last_reverse = scene_time - entity.last_recent_position_reverse_time;

    if (time_since_last_reverse > 0.035f) {
      entity.visible_position = entity.recent_positions.back();
      entity.recent_positions.pop_back();
      entity.last_recent_position_reverse_time = scene_time;
    }
  }

  static void SetMood(GameEntity& entity, EnemyMood mood, const float scene_time) {
    if (entity.enemy_state.mood != mood) {
      entity.enemy_state.mood = mood;
      entity.enemy_state.last_mood_change_time = scene_time;
    }
  }

  static void ResetEnemyState(EnemyState& enemy) {
    enemy.mood = ENEMY_IDLE;
    enemy.health = 100.f;
    enemy.speed = 0.f;
    enemy.last_mood_change_time = 0.f;
    enemy.last_attack_start_time = 0.f;
    enemy.last_attack_action_time = 0.f;
    enemy.last_block_time = 0.f;
    enemy.last_death_time = 0.f;
  }

  static void SoftResetEntity(GameEntity& entity, const float scene_time) {
    entity.visible_rotation = entity.orientation;

    ResetEnemyState(entity.enemy_state);
  }

  static void HardResetEntity(GameEntity& entity) {
    entity.visible_scale = tVec3f(0.f);
    entity.visible_position = entity.position;
    entity.visible_rotation = entity.orientation;

    entity.recent_positions.clear();

    ResetEnemyState(entity.enemy_state);
  }

  static void KillEnemy(GameEntity& entity, const float scene_time) {
    auto& enemy = entity.enemy_state;

    enemy.health = 0.f;

    enemy.last_death_time = scene_time;
    enemy.last_attack_start_time = 0.f;
    enemy.last_block_time = 0.f;
  }

  static tVec3f GetFacingDirection(GameEntity& entity) {
    // @todo why do we have to invert this?
    return entity.visible_rotation.getDirection().invert();
  }

  struct EntityProximity {
    float distance;
    float facing_dot;
  };

  static EntityProximity GetEntityProximity(GameEntity& entity, const State& state) {
    tVec3f player_to_entity = entity.position - state.player_position;
    float entity_distance = player_to_entity.magnitude();
    tVec3f unit_player_to_entity = player_to_entity / entity_distance;
    float facing_dot = tVec3f::dot(unit_player_to_entity, state.player_facing_direction);

    EntityProximity proximity;
    proximity.distance = entity_distance;
    proximity.facing_dot = facing_dot;

    return proximity;
  }

  static inline bool IsDuringActiveTime(const GameEntity& entity, const State& state) {
    return (
      state.astro_time >= entity.astro_start_time &&
      state.astro_time <= entity.astro_end_time
    );
  }
}