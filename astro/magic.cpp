#include "astro/magic.h"
#include "astro/entity_manager.h"
#include "astro/entity_dispatcher.h"
#include "astro/entity_behaviors/Lamppost.h"
#include "astro/entity_behaviors/Sculpture_1.h"

using namespace astro;

static EntityRecord FindWandActionTarget(State& state) {
  EntityRecord target;
  float closest_distance = FLT_MAX;

  // Lampposts
  {
    for_entities_of_type(LAMPPOST) {
      auto& entity = entities[i];
      auto proximity = GetEntityProximity(entity, state);

      if (
        proximity.distance < 9000.f &&
        proximity.facing_dot > 0.1f &&
        proximity.distance < closest_distance
      ) {
        target = GetRecord(entity);
        closest_distance = proximity.distance;
      }
    }
  }

  // Sculptures
  {
    for_entities_of_type(SCULPTURE_1) {
      auto& entity = entities[i];
      auto proximity = GetEntityProximity(entity, state);

      if (
        proximity.distance < 9000.f &&
        proximity.facing_dot > -0.5f &&
        proximity.distance < closest_distance
      ) {
        target = GetRecord(entity);
        closest_distance = proximity.distance;
      }
    }
  }

  return target;
}

static bool CanPerformWandAction(Tachyon* tachyon, State& state) {
  if (time_since(state.last_wand_strike_time) < 2.f) return false;
  if (time_since(state.last_wand_action_time) < 0.5f) return false;

  // If we have any targets or potential targets, we need to
  // check whether wand actions are allowed
  if (state.has_target || state.targetable_entities.size() > 0) {
    for (auto& record : state.targetable_entities) {
      if (record.type == FAERIE) {
        // If a Faerie is targeted, allow wand actions. Other enemy types
        // disable them to avoid inadvertently spamming lampposts or other
        // entities on and off.
        return true;
      }
    }

    return false;
  }

  // No targets!
  return true;
}

// @deprecated
void Magic::HandleWandAction(Tachyon* tachyon, State& state) {
  if (!CanPerformWandAction(tachyon, state)) return;

  state.last_wand_action_time = get_scene_time();

  auto target = FindWandActionTarget(state);

  if (target.type == UNSPECIFIED) {
    return;
  }

  auto& entity = *EntityManager::FindEntity(state, target);

  if (state.astro_time > entity.astro_end_time) {
    // Disallow wand actions on expired entities
    return;
  }

  switch (entity.type) {
    case LAMPPOST:
      Lamppost::HandleWandAction(tachyon, state, entity);
      break;
  }
}