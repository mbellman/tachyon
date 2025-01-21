#include "cosmodrone/autopilot.h"
#include "cosmodrone/piloting.h"

using namespace Cosmodrone;

void Piloting::HandlePiloting(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  // Handle flight system changes/piloting state
  {
    if (Autopilot::IsDocked(state) && !state.is_piloting_vehicle) {
      if (state.docking_target.mesh_index == meshes.fighter) {
        state.flight_system = FlightSystem::FIGHTER;
        state.is_piloting_vehicle = true;
        state.piloted_vehicle = state.docking_target;
      }
    } else if (!Autopilot::IsDocked(state) && state.is_piloting_vehicle) {
      state.flight_system = FlightSystem::DRONE;
      state.is_piloting_vehicle = false;
    }
  }
}