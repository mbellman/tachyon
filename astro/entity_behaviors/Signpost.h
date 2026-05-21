#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior Signpost {
    addMeshes() {
      meshes.signpost_placeholder = MODEL_MESH("./astro/3d_models/signpost/placeholder.obj", 500);
      meshes.signpost = MODEL_MESH("./astro/3d_models/signpost/signpost.obj", 500);
    }

    getMeshes() {
      return_meshes({
        meshes.signpost
      });
    }

    getPlaceholderMesh() {
      return meshes.signpost_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      reset_instances(meshes.signpost);

      // @todo astro time check
      for (auto& entity : state.signposts) {
        if (abs(state.player_position.x - entity.position.x) > 25000.f) continue;
        if (abs(state.player_position.z - entity.position.z) > 25000.f) continue;

        auto proximity = GetEntityProximity(entity, state);

        if (proximity.distance < 4000.f && proximity.facing_dot > 0.5f) {
          UISystem::ShowTransientDialogue(tachyon, state, "[X] Read");

          if (
            did_press_key(tKey::CONTROLLER_A) &&
            !state.has_blocking_dialogue
          ) {
            // @todo
          }
        }

        // Signpost
        {
          auto& signpost = use_instance(meshes.signpost);

          Sync(signpost, entity);

          signpost.color = tVec3f(1.f, 0.8f, 0.4f);
          signpost.material = tVec4f(0.8f, 0, 0, 0.6f);

          commit(signpost);
        }
      }
    }
  };
}
