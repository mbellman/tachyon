#pragma once

#include <map>
#include <string>
#include <vector>

#include "astro/game_state.h"
#include "astro/entity_types.h"

namespace astro {
  struct DecorativeMesh {
    uint16 mesh_index;
    std::string mesh_name;
    tVec3f default_scale;
    tVec3f default_color;
  };

  /**
   * ----------------------------
   * Returns the default properties for a given entity type.
   * ----------------------------
   */
  static EntityDefaults& GetEntityDefaults(EntityType type) {
    return entity_defaults_map.at(type);
  }

  /**
   * ----------------------------
   * Returns a list of all decorative meshes.
   * ----------------------------
   */
  static std::vector<DecorativeMesh>& GetDecorativeMeshes(State& state) {
    auto& meshes = state.meshes;

    static std::vector<DecorativeMesh> decorative_meshes = {
      {
        .mesh_index = meshes.rock_1,
        .mesh_name = "rock_1",
        .default_scale = tVec3f(500.f),
        .default_color = tVec3f(0.2f)
      },
      {
        .mesh_index = meshes.ground_1,
        .mesh_name = "ground_1",
        .default_scale = tVec3f(2000.f),
        .default_color = tVec3f(0.4f, 0.7f, 0.2f)
      },
      {
        .mesh_index = meshes.flat_ground,
        .mesh_name = "flat_ground",
        .default_scale = tVec3f(5000.f, 1.f, 5000.f),
        .default_color = tVec3f(0.4f, 0.5f, 0.1f)
      }
    };

    return decorative_meshes;
  }
}