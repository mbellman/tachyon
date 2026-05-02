
#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior CastleRampart {
    addMeshes() {
      meshes.castle_rampart_placeholder = CUBE_MESH(500);
      meshes.castle_rampart = CUBE_MESH(500);
    }

    getMeshes() {
      return_meshes({
        meshes.castle_rampart
      });
    }

    getPlaceholderMesh() {
      return meshes.castle_rampart_placeholder;
    }

    timeEvolve() {
      // @todo
    }
  };
}
  