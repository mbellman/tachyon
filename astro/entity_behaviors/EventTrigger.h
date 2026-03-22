#pragma once

#include "astro/entity_behaviors/behavior.h"
#include "astro/game_events.h"

namespace astro {
  behavior EventTrigger {
    addMeshes() {
      meshes.event_trigger_placeholder = SPHERE_MESH(500);

      mesh(meshes.event_trigger_placeholder).shadow_cascade_ceiling = 0;
    }

    getMeshes() {
      // Event triggers don't have a specific in-game object
      return_meshes({});
    }

    getPlaceholderMesh() {
      return meshes.event_trigger_placeholder;
    }

    timeEvolve() {
      for_entities(state.event_triggers) {
        auto& entity = state.event_triggers[i];

        // Do nothing if we've already triggered the event
        if (entity.did_activate) continue;

        // Do nothing if the trigger is unidentified
        if (entity.unique_name.empty()) continue;

        // Do nothing if the trigger is in a different time period
        if (!IsDuringActiveTime(entity, state)) continue;

        float player_distance = tVec3f::distance(state.player_position, entity.position);

        if (player_distance < 2000.f) {
          entity.did_activate = true;

          GameEvents::StartEvent(tachyon, state, entity.unique_name);
        }
      }
    }
  };
}