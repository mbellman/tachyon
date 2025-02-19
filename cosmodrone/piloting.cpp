#include "cosmodrone/autopilot.h"
#include "cosmodrone/piloting.h"
#include "cosmodrone/utilities.h"

using namespace Cosmodrone;

const static float START_UP_TIME = 2.f;

static float GetPilotingDuration(const State& state) {
  return state.current_game_time - state.piloting_start_time;
}

static void StartPiloting(Tachyon* tachyon, State& state) {
  state.is_piloting_vehicle = true;
  state.piloting_start_time = state.current_game_time;

  for (auto& vehicle : state.pilotable_vehicles) {
    if (vehicle.root_object == state.docking_target) {
      auto& live = *get_live_object(vehicle.root_object);

      state.current_piloted_vehicle = vehicle;
      state.current_piloted_vehicle.position = live.position;
      state.current_piloted_vehicle.rotation = live.rotation;

      break;
    }
  }
}

// @todo should ALL pilotable vehicles just continually update every frame?
// e.g. dealing with physics or other effectful interactions
static void UpdatePilotedVehicleParts(Tachyon* tachyon, State& state) {
  auto& vehicle = state.current_piloted_vehicle;

  for (auto& part : vehicle.parts) {
    auto& live = *get_live_object(part);

    live.position = vehicle.position;
    live.rotation = vehicle.rotation;

    if (live == vehicle.parts.back()) {
      live.color = tVec4f(1.f, 0.6f, 0.2f, state.jets_intensity);
    }

    commit(live);
  }
}

static void StartUpPilotedVehicle(Tachyon* tachyon, State& state, const float dt, const float duration) {
  auto& camera = tachyon->scene.camera;
  auto& root = *get_live_object(state.current_piloted_vehicle.root_object);
  auto up_direction = root.rotation.getUpDirection();
  auto& vehicle = state.current_piloted_vehicle;
  float up_speed = sinf(t_PI * duration / START_UP_TIME) * 20000.f;

  // Launch vehicle/ship together
  {
    state.current_piloted_vehicle.position += up_direction * up_speed * dt;

    UpdatePilotedVehicleParts(tachyon, state);

    state.docking_position = Autopilot::GetDockingPosition(tachyon, state);
    state.ship_position += up_direction * up_speed * dt;
    state.ship_position = tVec3f::lerp(state.ship_position, state.docking_position, dt);

  }

  // Orient the camera directly behind the ship
  {
    state.target_camera_rotation = vehicle.rotation.opposite();
  }
}

void Piloting::HandlePiloting(Tachyon* tachyon, State& state, const float dt) {
  auto& meshes = state.meshes;

  // Handle flight system changes/piloting state
  {
    if (Autopilot::IsDocked(state) && !state.is_piloting_vehicle) {
      if (state.docking_target.mesh_index == meshes.fighter_dock) {
        state.flight_system = FlightSystem::FIGHTER;

        StartPiloting(tachyon, state);
      }
    }
  }

  if (!state.is_piloting_vehicle) {
    return;
  }

  auto duration = GetPilotingDuration(state);

  if (duration < START_UP_TIME) {
    StartUpPilotedVehicle(tachyon, state, dt, duration);

    return;
  }

  // Control vehicle movement/position/rotation
  {
    auto& vehicle = state.current_piloted_vehicle;
    auto speed = state.ship_velocity.magnitude();

    vehicle.position += state.ship_velocity * dt;
    vehicle.rotation = objects(meshes.hull)[0].rotation;

    UpdatePilotedVehicleParts(tachyon, state);

    state.flight_mode = FlightMode::MANUAL_CONTROL;

    if (state.controlled_thrust_duration > 0.f) {
      if (state.controlled_thrust_duration < 1.f) {
        // Charging up
        state.ship_rotate_to_target_speed = 4.f;
      } else {
        // Flying
        float forward_dot = tVec3f::dot(state.view_forward_direction, state.ship_rotation_basis.forward);
        if (forward_dot < 0.f) forward_dot = 0.f;

        state.ship_rotate_to_target_speed = Tachyon_Lerpf(0.5f, 1.5f, forward_dot);
      }
    } else {
      // Stopped
      float speed_ratio = speed / Utilities::GetMaxShipSpeed(state);

      state.ship_rotate_to_target_speed = Tachyon_Lerpf(1.5f, 4.f, 1.f - speed_ratio);
    }

    // Lock drone to vehicle
    state.ship_position =
    state.docking_position = Autopilot::GetDockingPosition(tachyon, state);
  }
}