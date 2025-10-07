#pragma once

#include "engine/tachyon.h"
#include "astro/entities.h"
#include "astro/game_state.h"

// @todo move to engine
#define CUBE_MESH(total) Tachyon_AddMesh(tachyon, Tachyon_CreateCubeMesh(), total)
#define PLANE_MESH(total) Tachyon_AddMesh(tachyon, Tachyon_CreatePlaneMesh(), total)
#define MODEL_MESH(path, total) Tachyon_AddMesh(tachyon, Tachyon_LoadMesh(path), total)

#define return_meshes(...)\
  static std::vector<uint16> __mesh_list = __VA_ARGS__;\
  return __mesh_list;\

#define behavior struct
#define addMeshes() static void _AddMeshes(Tachyon* tachyon, MeshIds& meshes)
#define getMeshes() static const std::vector<uint16>& _GetMeshes(const MeshIds& meshes)
#define getPlaceholderMesh() static uint16 _GetPlaceholderMesh(const MeshIds& meshes)
#define timeEvolve() static void _TimeEvolve(Tachyon* tachyon, State& state, const float dt)
#define handleEnemyBehavior() static void _HandleEnemyBehavior(Tachyon* tachyon, State& state, GameEntity& entity, const float dt)

#define handle_enemy_behavior(__behavior) __behavior::_HandleEnemyBehavior(tachyon, state, entity, dt)

namespace astro {
  // @todo move this into engine
  static void RemoveLastObject(Tachyon* tachyon, uint16 mesh_index) {
    auto& objects = objects(mesh_index);
    uint16 total_active = objects.total_active;

    if (total_active > 0) {
      auto& last_object = objects[total_active - 1];
    
      remove(last_object);
    }
  }

  // @todo move elsewhere
  static float GetLivingEntityProgress(State& state, const GameEntity& entity, const float lifetime) {
    float entity_age = state.astro_time - entity.astro_start_time;
    if (entity_age < 0.f) return 0.f;
    if (entity_age > lifetime) return 1.f;

    return entity_age / lifetime;
  }

  static void Jitter(GameEntity& entity, const float amount) {
    entity.visible_position.x += Tachyon_GetRandom(-amount, amount);
    entity.visible_position.z += Tachyon_GetRandom(-amount, amount);
  }
}