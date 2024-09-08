#pragma once

#include "engine/tachyon.h"
#include "cosmodrone/game_types.h"

namespace Cosmodrone {
  namespace WorldSetup {
    void LoadMeshes(Tachyon* tachyon, MeshIds& meshes);
    void InitializeGameWorld(Tachyon* tachyon, MeshIds& meshes);
  }
}