#pragma once

#include "engine/tachyon.h"
#include "astro/entity_types.h"
#include "astro/game_state.h"

// @todo move to engine
#define CUBE_MESH(total) Tachyon_AddMesh(tachyon, Tachyon_CreateCubeMesh(), total)
#define PLANE_MESH(total) Tachyon_AddMesh(tachyon, Tachyon_CreatePlaneMesh(), total)
#define MODEL_MESH(path, total) Tachyon_AddMesh(tachyon, Tachyon_LoadMesh(path), total)

#define behavior struct
#define addMeshes() static void _AddMeshes(Tachyon* tachyon, State& state)
#define spawned() static void _SpawnObjects(Tachyon* tachyon, State& state)
#define destroyed() static void _DestroyObjects(Tachyon* tachyon, State& state)
#define createPlaceholder() static tObject& _CreatePlaceholder(Tachyon* tachyon, State& state, const GameEntity& entity)
#define destroyPlaceholders() static void _DestroyPlaceholders(Tachyon* tachyon, State& state)
#define timeEvolve() static void _TimeEvolve(Tachyon* tachyon, State& state)

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
}