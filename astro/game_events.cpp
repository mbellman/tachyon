#include "astro/game_events.h"
#include "astro/entity_behaviors/behavior.h"
#include "astro/entity_manager.h"
#include "astro/ui_system.h"

using namespace astro;

#define check_event_trigger(name, handler)\
  if (event_trigger == name) {\
    handler(tachyon, state);\
  }\

// @todo move to engine
template<class T>
static inline void RemoveFromArray(std::vector<T>& array, uint32 index) {
  array.erase(array.begin() + index);
}

struct EventSettings {
  float delay = 0.f;
  float duration = 1.f;
};

static void QueueEntityMoveEvent(Tachyon* tachyon, State& state, GameEntity& entity, const tVec3f& end_position, const EventSettings& settings) {
  EntityMoveEvent event;
  event.entity_record = GetRecord(entity);
  event.start_position = entity.visible_position;
  event.end_position = end_position;
  event.start_time = get_scene_time() + settings.delay;
  event.end_time = event.start_time + settings.duration;

  state.move_events.push_back(event);
}

static void QueueCameraTargetEvent(Tachyon* tachyon, State& state, GameEntity& target, const EventSettings& settings) {
  CameraEvent event;
  event.target_entity_record = GetRecord(target);
  event.start_time = get_scene_time() + settings.delay;
  event.end_time = event.start_time + settings.duration;

  state.camera_events.push_back(event);
}

/**
 * ------------------------
 * Event: Village Gate Open
 * ------------------------
 */
static void StartVillageGateEvent(Tachyon* tachyon, State& state) {
  for (auto& entity : state.npcs) {
    if (entity.unique_name == "gate_villager") {
      if (!IsDuringActiveTime(entity, state)) {
        // If we open the gate during a time when the gate villager
        // is not present, stop here and do nothing
        return;
      }

      QueueCameraTargetEvent(tachyon, state, entity, {
        .delay = 1.f,
        .duration = 1.f
      });
    }
  }

  UISystem::StartDialogueSet(state, "gate_villager_cutscene");
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
      tVec3f move_target_position = tVec3f(-125000.f, 0, 75000.f);

      QueueEntityMoveEvent(tachyon, state, entity, move_target_position, {
        .duration = 2.f
      });

      QueueCameraTargetEvent(tachyon, state, entity, {
        .duration = 3.f
      });

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
  check_event_trigger("village_gate", StartVillageGateEvent);
  check_event_trigger("river_wheel", StartRiverWheelEvent);
  check_event_trigger("bridge_open", StartBridgeOpenEvent);

  // @todo dev mode only
  {
    console_log("EVENT: " + event_trigger);
  }
}

void GameEvents::HandleEvents(Tachyon* tachyon, State& state) {
  float scene_time = get_scene_time();

  // Handle camera target events
  // @todo factor
  for (size_t i = 0; i < state.camera_events.size(); i++) {
    auto& event = state.camera_events[i];

    if (scene_time > event.end_time) {
      RemoveFromArray(state.camera_events, i);
    }
  }

  // Handle entity move events
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