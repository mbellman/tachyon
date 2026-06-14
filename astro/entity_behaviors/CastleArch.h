#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior CastleArch {
    addMeshes() {
      meshes.castle_arch_placeholder = MODEL_MESH("./astro/3d_models/castle_arch/placeholder.obj", 500);
      meshes.castle_arch = MODEL_MESH("./astro/3d_models/castle_arch/arch.obj", 500);
    }

    getMeshes() {
      return_meshes({
        meshes.castle_arch
      });
    }

    getPlaceholderMesh() {
      return meshes.castle_arch_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      reset_instances(meshes.castle_arch);

      for (auto& entity : state.castle_arches) {
        if (!IsDuringActiveTime(entity, state)) continue;
        // @todo range culling

        // Arch
        {
          auto& arch = use_instance(meshes.castle_arch);

          Sync(arch, entity);

          commit(arch);
        }
      }
    }
  };
}
