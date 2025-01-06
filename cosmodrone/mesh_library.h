#pragma once

#include "engine/tachyon.h"
#include "cosmodrone/game_types.h"

namespace Cosmodrone {
  struct MeshAsset {
    std::string mesh_name = "";
    uint16 mesh_index = 0;
    uint16 generated_from = 0;
    bool placeholder = false;
    bool moving = false;

    struct {
      tMeshType type = PBR_MESH;
      tVec3f scale = tVec3f(1000.f);
      tColor color = tVec3f(1.f);
      tMaterial material = tVec4f(0.6f, 0, 0, 0);
    } defaults;
  };

  namespace MeshLibrary {
    void LoadMeshes(Tachyon* tachyon, State& state);
    const std::vector<MeshAsset>& GetPlaceableMeshAssets();
    const std::vector<MeshAsset>& GetGeneratedMeshAssets();
    const MeshAsset& FindMeshAsset(uint16 mesh_index);
  }
}