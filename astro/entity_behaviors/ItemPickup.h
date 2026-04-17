#pragma once

#include "astro/entity_behaviors/behavior.h"
#include "astro/items.h"

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

        if (entity.did_activate) continue;
        if (!IsDuringActiveTime(entity, state)) continue;

        auto proximity = GetEntityProximity(entity, state);

        if (proximity.distance < 3000.f && proximity.facing_dot > 0.f) {
          UISystem::ShowTransientDialogue(tachyon, state, "[X] Pick up");

          if (did_press_key(tKey::CONTROLLER_A)) {
            auto item_type = Items::ItemNameToType(entity.item_pickup_name);

            Items::CollectItem(tachyon, state, item_type);

            // @temporary
            Sfx::PlaySound(SFX_SCULPTURE_ACTIVATE_1, 0.5f);

            entity.did_activate = true;
          }
        }

        // Pickup indicator
        {
          auto& pickup = use_instance(meshes.item_pickup);

          Sync(pickup, entity);

          pickup.scale = tVec3f(250.f);
          pickup.color = tVec4f(1.f);

          commit(pickup);
        }
      }
    }
  };
}