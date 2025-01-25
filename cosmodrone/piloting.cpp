#include "cosmodrone/autopilot.h"
#include "cosmodrone/piloting.h"

using namespace Cosmodrone;

const static float START_UP_TIME = 2.f;

static float GetPilotingDuration(const State& state) {
  return state.current_game_time - state.piloting_start_time;
}

static void StartPiloting(State& state) {
  state.is_piloting_vehicle = true;
  state.piloted_vehicle = state.docking_target;
  state.piloting_start_time = state.current_game_time;
}

static void StartUpPilotedVehicle(Tachyon* tachyon, State& state, const float dt, const float duration) {
  auto up_direction = state.piloted_vehicle.rotation.getUpDirection();
  auto& vehicle = *get_original_object(state.piloted_vehicle);
  float up_speed = sinf(t_PI * duration / START_UP_TIME) * 20000.f;

  vehicle.position += up_direction * up_speed * dt;

  state.ship_position += up_direction * up_speed * dt;
  state.docking_position = Autopilot::GetDockingPosition(tachyon, state);

  commit(vehicle);
}

void Piloting::HandlePiloting(Tachyon* tachyon, State& state, const float dt) {
  auto& meshes = state.meshes;

  // Handle flight system changes/piloting state
  {
    if (Autopilot::IsDocked(state) && !state.is_piloting_vehicle) {
      if (state.docking_target.mesh_index == meshes.fighter) {
        state.flight_system = FlightSystem::FIGHTER;

        StartPiloting(state);
      }
    }
  }

  if (!state.is_piloting_vehicle) {
    return;
  }

  auto& vehicle = *get_original_object(state.piloted_vehicle);
  auto duration = GetPilotingDuration(state);

  if (duration < START_UP_TIME) {
    StartUpPilotedVehicle(tachyon, state, dt, duration);

    return;
  }

  vehicle.position += state.ship_velocity * dt;
  vehicle.rotation = objects(meshes.hull)[0].rotation;

  state.flight_mode = FlightMode::MANUAL_CONTROL;
  state.ship_rotate_to_target_speed = 3.f;

  // Synchronize references (we probably shouldn't count on this)
  state.piloted_vehicle.position = vehicle.position;
  state.docking_target.position = vehicle.position;

  state.ship_position =
  state.docking_position = Autopilot::GetDockingPosition(tachyon, state);

  commit(vehicle);
}