#pragma once

#include "engine/tachyon.h"
#include "cosmodrone/game_types.h"

namespace Cosmodrone {
  struct MeshAsset {
    std::string mesh_name;
    uint16 mesh_index;

    struct {
      tVec3f scale;
      tMaterial material;
    } defaults = {
      .scale = tVec3f(1000.f),
      .material = tVec4f(0.6f, 0, 0, 0)
    };
  };

  namespace MeshLibrary {
    void LoadMeshes(Tachyon* tachyon, State& state);
    const std::vector<MeshAsset>& GetPlaceableMeshAssets();
  }
}