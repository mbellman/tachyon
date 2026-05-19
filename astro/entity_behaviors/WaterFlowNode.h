#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior WaterFlowNode {
    addMeshes() {
      meshes.water_flow_node_placeholder = SPHERE_MESH(500);
    }

    getMeshes() {
      return_meshes({});
    }

    getPlaceholderMesh() {
      return meshes.water_flow_node_placeholder;
    }

    timeEvolve() {
      // @todo
    }
  };
}
