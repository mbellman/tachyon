#pragma once

#include "engine/tachyon.h"
#include "astro/entities.h"
#include "astro/game_state.h"
#include "astro/ui_system.h"

// @todo move to engine
#define CUBE_MESH(total) Tachyon_AddMesh(tachyon, Tachyon_CreateCubeMesh(), total)
#define SPHERE_MESH(total) Tachyon_AddMesh(tachyon, Tachyon_CreateSphereMesh(20), total)
#define PLANE_MESH(total) Tachyon_AddMesh(tachyon, Tachyon_CreatePlaneMesh(), total)
#define MODEL_MESH(path, total) Tachyon_AddMesh(tachyon, Tachyon_LoadMesh(path), total)

#define behavior struct

#define return_meshes(...)\
  static std::vector<uint16> __mesh_list = __VA_ARGS__;\
  return __mesh_list;\

#define addMeshes() static void _AddMeshes(Tachyon* tachyon, MeshIds& meshes)
#define getMeshes() static const std::vector<uint16>& _GetMeshes(const MeshIds& meshes)
#define getPlaceholderMesh() static uint16 _GetPlaceholderMesh(const MeshIds& meshes)
#define timeEvolve() static void _TimeEvolve(Tachyon* tachyon, State& state, const float dt)
#define handleEnemyBehavior() static void _HandleEnemyBehavior(Tachyon* tachyon, State& state, GameEntity& entity, const float dt)

#define handle_enemy_behavior(__behavior) __behavior::_HandleEnemyBehavior(tachyon, state, entity, dt)

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

    // @todo use dt
    // @todo use nlerp
    entity.visible_rotation = Quaternion::slerp(entity.visible_rotation, facing_direction, 1.f / 60.f);
  }

  static void SetMood(GameEntity& entity, EnemyMood mood, const float scene_time) {
    if (entity.enemy_state.mood != mood) {
      entity.enemy_state.mood = mood;
      entity.enemy_state.last_mood_change_time = scene_time;
    }
  }

  static tVec3f GetFacingDirection(GameEntity& entity) {
    // @todo why do we have to invert this?
    return entity.visible_rotation.getDirection().invert();
  }

  static inline bool IsDuringActiveTime(const GameEntity& entity, const State& state) {
    return (
      state.astro_time >= entity.astro_start_time &&
      state.astro_time <= entity.astro_end_time
    );
  }
}