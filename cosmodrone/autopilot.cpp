#include "cosmodrone/autopilot.h"
#include "cosmodrone/drone_flight_system.h"
#include "cosmodrone/target_system.h"

using namespace Cosmodrone;

// @todo move to constants
const static auto FORWARD_VECTOR = tVec3f(0, 0, -1.f);
const static auto UP_VECTOR = tVec3f(0, 1.f, 0);
const static auto RIGHT_VECTOR = tVec3f(1.f, 0, 0);

const static float AUTO_DOCK_APPROACH_SPEED = 1500.f;
const static float AUTO_DOCK_APPROACH_SPEED_LIMIT = 2000.f;

// @todo move to utilities
static tVec3f GetDockingPositionOffset(const uint16 mesh_index, const State& state) {
  auto& meshes = state.meshes;

  if (mesh_index == meshes.antenna_3) {
    return tVec3f(0, -1.f, -1.f).unit() * 0.7f;
  }

  if (mesh_index == meshes.fighter_dock) {
    return tVec3f(0, 0.3f, 0.35f);
  }

  if (mesh_index == meshes.floater_1) {
    return tVec3f(0, 0.8f, 0);
  }

  return tVec3f(0.f);
}

// @todo move to utilities
static tVec3f GetDockingPositionOffset(const State& state) {
  return GetDockingPositionOffset(state.docking_target.mesh_index, state);
}

// @todo move to utilities
static Quaternion GetDockedRotation(const State& state, const uint16 mesh_index) {
  auto& meshes = state.meshes;

  if (
    mesh_index == meshes.antenna_3 ||
    mesh_index == meshes.floater_1
  ) {
    return Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), t_PI);
  }

  return Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), 0.f);
}

// @todo move to utilities
static Quaternion GetDockedCameraRotation(const State& state, const tObject& target) {
  auto& meshes = state.meshes;

  if (
    target.mesh_index == meshes.antenna_3 ||
    target.mesh_index == meshes.floater_1
  ) {
    return (
      Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), 0.6f) *
      Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), t_PI * 1.2f) *
      target.rotation.opposite()
    );
  }

  return (
    Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), 0.2f) *
    target.rotation.opposite()
  );
}

// @todo move to utilities
static float GetDockedCameraDistance(const State& state, const uint16 mesh_index) {
  auto& meshes = state.meshes;

  if (mesh_index == meshes.fighter_dock) {
    return 15000.f;
  }

  return 30000.f;
}

static void DecelerateRetrograde(State& state, const float dt) {
  // Figure out how 'backward' the ship is pointed
  float reverse_dot = tVec3f::dot(state.ship_rotation_basis.forward, state.retrograde_direction);

  // Only decelerate when sufficiently reversed, to minimize curving
  if (reverse_dot < -0.9999f) {
    // Decelerate proportional to current speed, with clamping
    float deceleration = state.ship_velocity.magnitude();
    if (deceleration > 10000.f) deceleration = 10000.f;
    if (deceleration < 1000.f) deceleration = 1000.f;

    DroneFlightSystem::ThrustForward(state, dt, deceleration);
  }
}

static void HandleDockingApproachCamera(Tachyon* tachyon, State& state, tObject& target, float docking_distance) {
  // @todo cache this when we start the docking stage
  auto docked_camera_rotation = GetDockedCameraRotation(state, target);

  // @todo use ease-in-out
  float camera_blend = powf(1.f - docking_distance / state.initial_approach_ship_distance, 2.f);
  // Slightly increase the blend factor so we always hit the mark
  camera_blend *= 1.1f;
  if (camera_blend > 1.f) camera_blend = 1.f;

  camera_blend = Tachyon_EaseInOutf(camera_blend);

  tachyon->scene.camera.rotation = state.target_camera_rotation = Quaternion::slerp(
    state.initial_approach_camera_rotation,
    docked_camera_rotation,
    camera_blend
  );

  state.ship_camera_distance = state.ship_camera_distance_target = Tachyon_Lerpf(
    state.initial_approach_camera_distance,
    GetDockedCameraDistance(state, state.docking_target.mesh_index),
    camera_blend
  );
}

// @todo Clean/split this function up. Too much being managed here.
void Autopilot::HandleAutopilot(Tachyon* tachyon, State& state, const float dt) {
  auto& camera = tachyon->scene.camera;

  switch (state.flight_mode) {
    case FlightMode::AUTO_RETROGRADE: {
      DecelerateRetrograde(state, dt);

      if (state.ship_velocity.magnitude() < 200.f) {
        // Restore manual control when sufficiently decelerated
        state.flight_mode = FlightMode::MANUAL_CONTROL;
        state.ship_rotate_to_target_speed = 0.f;
      }

      break;
    }

    case FlightMode::AUTO_DOCK: {
      if (state.auto_dock_stage == AutoDockStage::APPROACH_DECELERATION) {
        DecelerateRetrograde(state, dt);

        if (state.ship_velocity.magnitude() < 50.f) {
          state.auto_dock_stage = AutoDockStage::APPROACH_ALIGNMENT;
          state.ship_rotate_to_target_speed = 0.f;
        }
      }

      if (
        state.auto_dock_stage == AutoDockStage::APPROACH &&
        state.ship_velocity.magnitude() < AUTO_DOCK_APPROACH_SPEED_LIMIT
      ) {
        DroneFlightSystem::ThrustForward(state, dt, AUTO_DOCK_APPROACH_SPEED);
      }
    }
  }

  if (state.flight_mode == FlightMode::AUTO_PROGRADE) {
    state.target_ship_rotation = Quaternion::FromDirection(state.ship_velocity_basis.forward.invert(), state.ship_rotation_basis.up);
    state.ship_rotate_to_target_speed += 4.f * dt;
  }

  if (state.flight_mode == FlightMode::AUTO_RETROGRADE) {
    state.target_ship_rotation = Quaternion::FromDirection(state.retrograde_direction, state.ship_rotation_basis.up);
    state.ship_rotate_to_target_speed += 3.f * dt;
  }

  if (state.flight_mode == FlightMode::AUTO_DOCK) {
    auto& target = *get_original_object(state.docking_target);

    // @todo allow us to reuse target for this, rather than getting it again, redundantly
    state.docking_position = GetDockingPosition(tachyon, state);

    switch (state.auto_dock_stage) {
      case AutoDockStage::APPROACH_DECELERATION: {
        state.target_ship_rotation = Quaternion::FromDirection(state.retrograde_direction, state.ship_rotation_basis.up);
        state.ship_rotate_to_target_speed += 4.f * dt;

        break;
      }

      case AutoDockStage::APPROACH_ALIGNMENT: {
        auto target_object_rotation = target.rotation;
        auto forward = (state.docking_position - state.ship_position).unit();
        auto target_object_up = target_object_rotation.getUpDirection();

        state.target_ship_rotation = Quaternion::FromDirection(forward.invert(), target_object_up);

        if (GetDockingAlignment(state, state.docking_position) <= 0.99999f) {
          // Slow the ship down as we align it
          state.ship_velocity *= (1.f - 5.f * dt);
        } else {
          // Begin approach
          state.auto_dock_stage = AutoDockStage::APPROACH;

          state.initial_approach_camera_rotation = camera.rotation;
          state.initial_approach_camera_distance = state.ship_camera_distance;
          state.initial_approach_ship_distance = (state.docking_position - state.ship_position).magnitude();
        }

        break;
      }

      case AutoDockStage::APPROACH: {
        auto docking_distance = (state.ship_position - state.docking_position).magnitude();

        HandleDockingApproachCamera(tachyon, state, target, docking_distance);

        if (
          state.ship_velocity.magnitude() >= AUTO_DOCK_APPROACH_SPEED_LIMIT &&
          docking_distance < 10000.f
        ) {
          // Begin final docking maneuver
          state.auto_dock_stage = AutoDockStage::DOCKING;
          state.initial_docking_ship_rotation = objects(state.meshes.hull)[0].rotation;
          state.initial_docking_ship_distance = docking_distance;
        }

        break;
      }

      case AutoDockStage::DOCKING: {
        auto docking_distance = (state.ship_position - state.docking_position).magnitude();

        HandleDockingApproachCamera(tachyon, state, target, docking_distance);

        // Rotate the ship into the final docking position
        state.target_ship_rotation =
          target.rotation *
          GetDockedRotation(state, state.docking_target.mesh_index);

        // Rotate the ship into docking position
        {
          float rotation_factor = 1.f - docking_distance / state.initial_docking_ship_distance;
          // Slightly increase the rotation rate so we always hit the mark
          rotation_factor *= 1.1f;
          if (rotation_factor > 1.f) rotation_factor = 1.f;

          auto& hull = objects(state.meshes.hull)[0];

          hull.rotation = Quaternion::slerp(state.initial_docking_ship_rotation, state.target_ship_rotation, rotation_factor);
        }

        // Slow down as we make the final approach
        if (docking_distance < 5000.f) {
          float speed = AUTO_DOCK_APPROACH_SPEED_LIMIT * (docking_distance / 5000.f);

          // Slow the ship as it docks
          state.ship_velocity = state.ship_velocity_basis.forward * speed;
        }

        // @todo @bug Ensure we never miss the target;
        // this seems to happen once in a blue moon!
        if (docking_distance < 300.f) {
          // Dock the ship
          state.auto_dock_stage = AutoDockStage::DOCKED;
          state.ship_velocity = 0.f;
          state.ship_rotate_to_target_speed = 0.f;
        }
      }
    }
  }
}

bool Autopilot::IsAutopilotActive(const State& state) {
  return state.flight_mode != FlightMode::MANUAL_CONTROL;
}

bool Autopilot::AttemptDockingProcedure(State& state) {
  auto* tracker = TargetSystem::GetSelectedTargetTracker(state);

  if (tracker == nullptr) {
    return false;
  }

  // @todo use live object (or ensure tracker->object is always live)
  auto& target_object = tracker->object;
  auto& ship_position = state.ship_position;
  auto target_distance = (target_object.position - ship_position).magnitude();

  if (target_distance > 100000.f) {
    return false;
  }

  state.flight_mode = FlightMode::AUTO_DOCK;
  state.docking_target = target_object;
  state.ship_rotate_to_target_speed = 0.f;
  // @todo define the retrograde direction correctly (as the anti-vector of velocity).
  // We're doing this for now because of quirks with the player drone model, which should
  // probably be correctly oriented.
  state.retrograde_direction = state.ship_velocity_basis.forward;

  if (state.ship_velocity.magnitude() < 3000.f) {
    state.auto_dock_stage = AutoDockStage::APPROACH_ALIGNMENT;
  } else {
    state.auto_dock_stage = AutoDockStage::APPROACH_DECELERATION;
  }

  return true;
}

bool Autopilot::IsDoingDockingApproach(const State& state) {
  return state.flight_mode == FlightMode::AUTO_DOCK && state.auto_dock_stage >= AutoDockStage::APPROACH;
}

bool Autopilot::IsDoingDockingAlignment(const State& state) {
  return state.flight_mode == FlightMode::AUTO_DOCK && state.auto_dock_stage == AutoDockStage::APPROACH_ALIGNMENT;
}

bool Autopilot::IsDocked(const State& state) {
  return (
    (state.flight_mode == FlightMode::AUTO_DOCK || state.is_piloting_vehicle) &&
    state.auto_dock_stage == AutoDockStage::DOCKED
  );
}

tVec3f Autopilot::GetDockingPosition(Tachyon* tachyon, const State& state) {
  auto* target = get_original_object(state.docking_target);

  if (target == nullptr) {
    return tVec3f(0.f);
  }

  auto& target_rotation = target->rotation;
  auto offset = GetDockingPositionOffset(state);

  offset *= target->scale;
  offset = target_rotation.toMatrix4f() * offset;

  return target->position + offset;
}

tVec3f Autopilot::GetDockingPosition(Tachyon* tachyon, const State& state, const tObject& object) {
  auto* target = get_original_object(object);

  if (target == nullptr) {
    return tVec3f(0.f);
  }

  auto& target_rotation = target->rotation;
  auto offset = GetDockingPositionOffset(object.mesh_index, state);

  offset *= target->scale;
  offset = target_rotation.toMatrix4f() * offset;

  return target->position + offset;
}

float Autopilot::GetDockingAlignment(const State& state, const tVec3f& docking_position) {
  auto forward = state.ship_rotation_basis.forward;
  auto ship_to_target = (docking_position - state.ship_position).unit();

  return tVec3f::dot(forward, ship_to_target);
}

void Autopilot::Undock(Tachyon* tachyon, State& state) {
  auto* docked_target = get_original_object(state.docking_target);

  if (docked_target == nullptr) {
    return;
  }

  auto& hull = objects(state.meshes.hull)[0];

  state.flight_system = FlightSystem::DRONE;
  state.flight_mode = FlightMode::MANUAL_CONTROL;
  state.is_piloting_vehicle = false;
  state.ship_camera_distance_target = 1800.f;
  state.target_camera_rotation = hull.rotation.opposite();

  state.ship_velocity = (
    state.ship_rotation_basis.forward.invert() * 5.f +
    state.ship_rotation_basis.up
  ).unit() * 500.f;
}