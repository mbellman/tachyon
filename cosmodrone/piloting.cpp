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

    // Handle jets
    if (live == vehicle.parts.back()) {
      float jets_intensity = 0.5f + 0.5f * state.jets_intensity;
      if (jets_intensity > 0.98f) jets_intensity = 1.f;

      live.color = tVec4f(1.f, 0.6f, 0.2f, jets_intensity);
    }

    commit(live);
  }

  // @todo factor
  if (vehicle.root_object.mesh_index == state.meshes.fighter_dock) {
    float retraction = 0.32f * powf(Tachyon_EaseInOutf(state.jets_intensity), 3.f);
    auto x_axis = vehicle.rotation.getLeftDirection();

    auto& left_wing = *get_live_object(vehicle.parts[5]);
    auto& left_wing_turrets = *get_live_object(vehicle.parts[6]);
    auto& right_wing = *get_live_object(vehicle.parts[7]);
    auto& right_wing_turrets = *get_live_object(vehicle.parts[8]);

    left_wing.position -= x_axis * retraction * left_wing.scale.x;
    left_wing_turrets.position = left_wing.position;
    right_wing.position += x_axis * retraction * right_wing.scale.x;
    right_wing_turrets.position = right_wing.position;

    commit(left_wing);
    commit(left_wing_turrets);
    commit(right_wing);
    commit(right_wing_turrets);
  }
}

static void StartUpPilotedVehicle(Tachyon* tachyon, State& state, const float dt, const float duration) {
  if (duration < 1.f) {
    return;
  }

  auto& camera = tachyon->scene.camera;
  auto& root = *get_live_object(state.current_piloted_vehicle.root_object);
  auto up_direction = root.rotation.getUpDirection();
  auto& vehicle = state.current_piloted_vehicle;
  float up_speed = sinf(t_TAU * (duration - 1.f) / START_UP_TIME) * 40000.f;

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

static void HandleQuickReversal(Tachyon* tachyon, State& state, const float dt) {
  float alpha = 4.f * (state.current_game_time - state.last_fighter_reversal_time);
  if (alpha > 1.f) alpha = 1.f;
  alpha *= alpha;

  state.target_ship_rotation = Quaternion::FromDirection(state.retrograde_direction, state.retrograde_up);
  state.ship_rotate_to_target_speed = 3.f * alpha;
}

static void HandleQuickTarget() {
  // @todo
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

      if (state.docking_target.mesh_index == meshes.freight_dock) {
        // @todo add a new flight system type
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

    // Lock drone to vehicle
    state.ship_position =
    state.docking_position = Autopilot::GetDockingPosition(tachyon, state);

    if (
      state.current_game_time - state.last_fighter_reversal_time < 2.5f &&
      state.flight_mode == FlightMode::AUTO_RETROGRADE
    ) {
      HandleQuickReversal(tachyon, state, dt);

      return;
    }

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
  }
}