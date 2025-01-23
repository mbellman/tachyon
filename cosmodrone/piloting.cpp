#include "cosmodrone/autopilot.h"
#include "cosmodrone/piloting.h"

using namespace Cosmodrone;

const static float START_UP_TIME = 1.f;

static float GetPilotingDuration(const State& state) {
  return state.current_game_time - state.piloting_start_time;
}

static void StartPiloting(State& state) {
  state.is_piloting_vehicle = true;
  state.piloted_vehicle = state.docking_target;
  state.piloting_start_time = state.current_game_time;
}

static void StopPiloting(State& state) {
  state.flight_system = FlightSystem::DRONE;
  state.is_piloting_vehicle = false;
}

static void StartUpPilotedVehicle(Tachyon* tachyon, State& state, const float dt, const float duration) {
  auto up_direction = state.piloted_vehicle.rotation.getUpDirection();
  auto& vehicle = *get_original_object(state.piloted_vehicle);
  float blend = (START_UP_TIME - duration) / START_UP_TIME;
  float up_speed = powf(blend, 2.f) * 50000.f;

  state.ship_position += up_direction * up_speed * dt;

  vehicle.position += up_direction * up_speed * dt;

  commit(vehicle);
}

void Piloting::HandlePiloting(Tachyon* tachyon, State& state, const float dt) {
  auto& meshes = state.meshes;

  // Handle flight system changes/piloting state
  {
    if (Autopilot::IsDocked(state) && !state.is_piloting_vehicle) {
      StartPiloting(state);

      if (state.docking_target.mesh_index == meshes.fighter) {
        state.flight_system = FlightSystem::FIGHTER;
      }
    } else if (!Autopilot::IsDocked(state) && state.is_piloting_vehicle) {
      StopPiloting(state);
    }
  }

  // Start up sequence
  {
    auto duration = GetPilotingDuration(state);

    if (state.is_piloting_vehicle && duration < START_UP_TIME) {
      StartUpPilotedVehicle(tachyon, state, dt, duration);
    }
  }
}