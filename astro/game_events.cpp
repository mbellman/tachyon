#include "astro/game_events.h"
#include "astro/ui_system.h"

using namespace astro;

#define process_event(name, handler)\
  if (event_trigger == name) {\
    return handler(tachyon, state);\
  }\

static void RiverWheelEvent(Tachyon* tachyon, State& state) {
  for (auto& npc : state.npcs) {
    if (npc.unique_name == "dweller_river") {
      // @todo
    }
  }

  UISystem::ShowBlockingDialogue(tachyon, state, "Hey! What gives?");
}

void GameEvents::ProcessEvent(Tachyon* tachyon, State& state, const std::string& event_trigger) {
  process_event("river_wheel", RiverWheelEvent);
}