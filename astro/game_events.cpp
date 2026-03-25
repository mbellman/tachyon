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
  float blend_factor = 1.f;
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
  event.blend_factor = settings.blend_factor;

  state.camera_events.push_back(event);
}

/**
 * -------------------------
 * Event: Village Gate Guard
 * -------------------------
 */
static void StartVillageGateGuardEvent(Tachyon* tachyon, State& state) {
  for (auto& entity : state.low_guards) {
    if (entity.unique_name == "gate_guard") {
      QueueCameraTargetEvent(tachyon, state, entity, {
        .delay = 0.65f,
        .duration = 2.f,
        .blend_factor = 0.5f
      });
    }
  }
}

/**
 * ------------------------
 * Event: Village Gate Open
 * ------------------------
 */
static void StartVillageGateOpenEvent(Tachyon* tachyon, State& state) {
  for (auto& entity : state.npcs) {
    if (entity.unique_name == "gate_villager") {
      if (!IsDuringActiveTime(entity, state)) {
        // If we open the gate during a time when the gate villager
        // is not present, stop here and do nothing
        return;
      }

      QueueCameraTargetEvent(tachyon, state, entity, {
        .delay = 0.f,
        .duration = 2.f
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
      tVec3f move_target_position = tVec3f(-50000.f, 0, 225000.f);

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
  check_event_trigger("village_gate_guard", StartVillageGateGuardEvent);
  check_event_trigger("village_gate", StartVillageGateOpenEvent);
  check_event_trigger("river_wheel", StartRiverWheelEvent);
  check_event_trigger("bridge_open", StartBridgeOpenEvent);

  // @todo dev mode only
  {
    console_log("EVENT: " + event_trigger);
  }
}

void GameEvents::HandleEvents(Tachyon* tachyon, State& state) {
  float scene_time = get_scene_time();

  // Manage camera events. Actual handling is in
  // CameraSystem::UpdateCamera() -> UpdateEventCamera().
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