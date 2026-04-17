#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior ItemPickup {
    addMeshes() {
      meshes.item_pickup_placeholder = SPHERE_MESH(500);
      // @temporary
      meshes.item_pickup = CUBE_MESH(500);

      mesh(meshes.item_pickup_placeholder).shadow_cascade_ceiling = 0;
      mesh(meshes.item_pickup).shadow_cascade_ceiling = 0;
    }

    getMeshes() {
      return_meshes({
        meshes.item_pickup
      });
    }

    getPlaceholderMesh() {
      return meshes.item_pickup_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      reset_instances(meshes.item_pickup);

      // @todo culling
      for_entities(state.item_pickups) {
        auto& entity = state.item_pickups[i];

        if (!IsDuringActiveTime(entity, state)) continue;

        auto proximity = GetEntityProximity(entity, state);

        if (proximity.distance < 3000.f && proximity.facing_dot > 0.f) {
          UISystem::ShowTransientDialogue(tachyon, state, "[X] Pick up");
        }

        // Pickup indicator
        {
          auto& pickup = use_instance(meshes.item_pickup);

          Sync(pickup, entity);

          pickup.scale = tVec3f(500.f);
          pickup.color = tVec4f(1.f);

          commit(pickup);
        }
      }
    }
  };
}