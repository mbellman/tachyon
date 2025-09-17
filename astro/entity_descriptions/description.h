#pragma once

#include "engine/tachyon.h"
#include "astro/entities.h"
#include "astro/game_state.h"

#define description struct
#define created() static void _CreateObjects(Tachyon* tachyon, State& state)
#define destroyed() static void _DestroyObjects(Tachyon* tachyon, State& state)
#define placeholderCreated() static tObject& _SpawnPlaceholder(Tachyon* tachyon, State& state, const GameEntity& entity)
#define placeholdersDestroyed() static void _DestroyPlaceholders(Tachyon* tachyon, State& state)

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
}