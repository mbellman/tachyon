#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior Npc {
    addMeshes() {
      meshes.npc_placeholder = MODEL_MESH("./astro/3d_models/guy.obj", 500);
      meshes.npc = MODEL_MESH("./astro/3d_models/guy.obj", 500);
    }

    getMeshes() {
      // Path nodes don't have a specific in-game object;
      // path segments are generated between them.
      return_meshes({
        meshes.npc
      });
    }

    getPlaceholderMesh() {
      return meshes.npc_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      for_entities(state.npcs) {
        auto& entity = state.npcs[i];

        // Body
        {
          auto& body = objects(meshes.npc)[0];

          Sync(body, entity);

          commit(body);
        }
      }
    }
  };
}