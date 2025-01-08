#pragma once

#include "cosmodrone/game_types.h"

namespace Cosmodrone {
  namespace ProceduralGeneration {
    void LoadMeshes(Tachyon* tachyon, State& state);
    void RemoveAutoPlacedObjects(Tachyon* tachyon, State& state);
    void GenerateWorld(Tachyon* tachyon, State& state);
  }
}