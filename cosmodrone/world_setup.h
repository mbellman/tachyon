#pragma once

#include "engine/tachyon.h"

namespace Cosmodrone {
  struct MeshIds {
    uint32
      // Ship parts
      hull,
      streams,
      thrusters,
      trim,

      // Celestial bodies
      planet,

      // Debug meshes
      sphere,
      cube;
  };

  namespace WorldSetup {
    void LoadMeshes(Tachyon* tachyon, MeshIds& meshes);
    void InitializeGameWorld(Tachyon* tachyon, MeshIds& meshes);
  }
}