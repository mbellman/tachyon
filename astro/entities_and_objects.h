#pragma once

#include <map>
#include <string>
#include <vector>

#include "astro/game_state.h"
#include "astro/entities.h"

// @todo rename object_lookup or similar

namespace astro {
  struct DecorativeMesh {
    uint16 mesh_index;
    std::string mesh_name;
    tVec3f default_scale;
    tVec3f default_color;
  };

  static std::string unknown_name = "Unknown";

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
        .default_color = tVec3f(0.3f)
      },
      {
        .mesh_index = meshes.river_edge,
        .mesh_name = "river_edge",
        .default_scale = tVec3f(1000.f),
        .default_color = tVec3f(0.27f, 0.135f, 0.135f)
      },
      {
        .mesh_index = meshes.ground_1,
        .mesh_name = "ground_1",
        .default_scale = tVec3f(2000.f),
        .default_color = tVec3f(0.1f, 0.2f, 0.1f)
      },
      {
        .mesh_index = meshes.flat_ground,
        .mesh_name = "flat_ground",
        .default_scale = tVec3f(5000.f, 1.f, 5000.f),
        .default_color = tVec3f(0.3f, 0.5f, 0.1f)
      },
      {
        .mesh_index = meshes.lookout_tower,
        .mesh_name = "lookout_tower",
        .default_scale = tVec3f(4000.f),
        .default_color = tVec3f(0.8f)
      },
    };

    return decorative_meshes;
  }

  /**
   * ----------------------------
   * Returns the name of a given mesh.
   * ----------------------------
   */
  static std::string& GetDecorativeMeshName(State& state, uint16 mesh_index) {
    for (auto& mesh : GetDecorativeMeshes(state)) {
      if (mesh.mesh_index == mesh_index) {
        return mesh.mesh_name;
      }
    }

    return unknown_name;
  }
}