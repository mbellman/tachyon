#include "astro/entity_manager.h"
#include "astro/game_events.h"
#include "astro/ui_system.h"

using namespace astro;

#define process_event(name, handler)\
  if (event_trigger == name) {\
    return handler(tachyon, state);\
  }\

static void AddMoveEvent(Tachyon* tachyon, State& state, GameEntity& entity, const tVec3f& end_position, const float duration) {
  EntityMoveEvent event;
  event.entity_record = GetRecord(entity);
  event.start_position = entity.visible_position;
  event.end_position = end_position;
  event.start_time = get_scene_time();
  event.end_time = event.start_time + duration;

  state.move_events.push_back(event);
}

/**
 * ------------------
 * Event: River Wheel
 * ------------------
 */
static void RiverWheelEvent(Tachyon* tachyon, State& state) {
  for (auto& entity : state.npcs) {
    if (entity.unique_name == "dweller_river") {
      // @TEMPORARY!!!!
      tVec3f end_position = tVec3f(-125000.f, 0, 75000.f);

      AddMoveEvent(tachyon, state, entity, end_position, 2.f);

      break;
    }
  }

  UISystem::ShowBlockingDialogue(tachyon, state, "Hey! What gives?");
}

/**
 * ------------------
 */

void GameEvents::StartEvent(Tachyon* tachyon, State& state, const std::string& event_trigger) {
  process_event("river_wheel", RiverWheelEvent);
}

void GameEvents::HandleEvents(Tachyon* tachyon, State& state) {
  float scene_time = get_scene_time();

  for (size_t i = 0; i < state.move_events.size(); i++) {
    auto& event = state.move_events[i];

    if (scene_time > event.end_time) {
      // @todo remove event
    } else {
      auto& entity = *EntityManager::FindEntity(state, event.entity_record);
      float alpha = Tachyon_InverseLerp(event.start_time, event.end_time, scene_time);
      tVec3f current_position = tVec3f::lerp(event.start_position, event.end_position, alpha);

      entity.visible_position = current_position;
    }
  }
}