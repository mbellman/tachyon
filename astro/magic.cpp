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

void Magic::HandleWandAction(Tachyon* tachyon, State& state) {
  if (time_since(state.last_wand_action_time) < 0.5f || state.has_target) {
    return;
  }

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
    case SCULPTURE_1:
      Sculpture_1::HandleWandAction(tachyon, state, entity);
      break;
  }
}