#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior HeavySwitch {
    addMeshes() {
      meshes.heavy_switch_placeholder = MODEL_MESH("./astro/3d_models/heavy_switch/placeholder.obj", 500);
      meshes.heavy_switch_base = MODEL_MESH("./astro/3d_models/heavy_switch/base.obj", 500);
      meshes.heavy_switch_button = MODEL_MESH("./astro/3d_models/heavy_switch/button.obj", 500);
    }

    getMeshes() {
      return_meshes({
        meshes.heavy_switch_base,
        meshes.heavy_switch_button
      });
    }

    getPlaceholderMesh() {
      return meshes.heavy_switch_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      reset_instances(meshes.heavy_switch_base);
      reset_instances(meshes.heavy_switch_button);

      for (auto& entity : state.heavy_switches) {
        // Base
        {
          auto& base = use_instance(meshes.heavy_switch_base);

          Sync(base, entity);

          commit(base);
        }

        // Switch
        {
          auto& button = use_instance(meshes.heavy_switch_button);

          Sync(button, entity);

          commit(button);
        }
      }
    }
  };
}
