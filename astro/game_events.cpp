#include "astro/entity_manager.h"
#include "astro/game_events.h"
#include "astro/ui_system.h"

using namespace astro;

#define process_event(name, handler)\
  if (event_trigger == name) {\
    return handler(tachyon, state);\
  }\

// @todo move to engine
template<class T>
static inline void RemoveFromArray(std::vector<T>& array, uint32 index) {
  array.erase(array.begin() + index);
}

static void AddMoveEvent(Tachyon* tachyon, State& state, GameEntity& entity, const tVec3f& end_position, const float duration) {
  EntityMoveEvent event;
  event.entity_record = GetRecord(entity);
  event.start_position = entity.visible_position;
  event.end_position = end_position;
  event.start_time = get_scene_time();
  event.end_time = event.start_time + duration;

  state.move_events.push_back(event);
}

static void AddCameraEvent(Tachyon* tachyon, State& state, GameEntity& target, const float duration) {
  CameraEvent event;
  event.target_entity_record = GetRecord(target);
  event.start_time = get_scene_time();
  event.end_time = event.start_time + duration;

  state.camera_events.push_back(event);
}

/**
 * ------------------
 * Event: River Wheel
 * ------------------
 */
static void StartRiverWheelEvent(Tachyon* tachyon, State& state) {
  for (auto& entity : state.npcs) {
    if (entity.unique_name == "dweller_river") {
      // @TEMPORARY!!!!
      tVec3f end_position = tVec3f(-125000.f, 0, 75000.f);

      AddMoveEvent(tachyon, state, entity, end_position, 2.f);
      AddCameraEvent(tachyon, state, entity, 3.f);

      break;
    }
  }

  UISystem::StartDialogueSet(state, "dweller_river_cutscene");
}

/**
 * ------------------
 * Event: Bridge Open
 * ------------------
 */
static void StartBridgeOpenEvent(Tachyon* tachyon, State& state) {
  // @todo
}

/**
 * ------------------
 */

void GameEvents::StartEvent(Tachyon* tachyon, State& state, const std::string& event_trigger) {
  process_event("river_wheel", StartRiverWheelEvent);
  process_event("bridge_open", StartBridgeOpenEvent);
}

void GameEvents::HandleEvents(Tachyon* tachyon, State& state) {
  float scene_time = get_scene_time();

  // @todo factor
  for (size_t i = 0; i < state.camera_events.size(); i++) {
    auto& event = state.camera_events[i];

    if (scene_time > event.end_time) {
      RemoveFromArray(state.camera_events, i);
    }
  }

  // @todo factor
  for (size_t i = 0; i < state.move_events.size(); i++) {
    auto& event = state.move_events[i];

    if (scene_time > event.end_time) {
      RemoveFromArray(state.move_events, i);
    } else {
      auto& entity = *EntityManager::FindEntity(state, event.entity_record);
      float alpha = Tachyon_InverseLerp(event.start_time, event.end_time, scene_time);
      tVec3f current_position = tVec3f::lerp(event.start_position, event.end_position, alpha);

      entity.visible_position = current_position;
    }
  }
}