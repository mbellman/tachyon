#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior NormalSwitch {
    addMeshes() {
      meshes.normal_switch_placeholder = MODEL_MESH("./astro/3d_models/normal_switch/placeholder.obj", 500);
      meshes.normal_switch_base = MODEL_MESH("./astro/3d_models/normal_switch/base.obj", 500);
      meshes.normal_switch_button = MODEL_MESH("./astro/3d_models/normal_switch/button.obj", 500);
    }

    getMeshes() {
      return_meshes({
        meshes.normal_switch_base,
        meshes.normal_switch_button
      });
    }

    getPlaceholderMesh() {
      return meshes.normal_switch_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      reset_instances(meshes.normal_switch_base);
      reset_instances(meshes.normal_switch_button);

      for (auto& entity : state.normal_switches) {
        // Base
        {
          auto& base = use_instance(meshes.normal_switch_base);

          Sync(base, entity);

          commit(base);
        }

        // Switch
        {
          auto& button = use_instance(meshes.normal_switch_button);

          Sync(button, entity);

          commit(button);
        }
      }
    }
  };
}
