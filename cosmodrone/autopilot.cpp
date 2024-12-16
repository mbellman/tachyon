#include "cosmodrone/autopilot.h"
#include "cosmodrone/flight_system.h"

using namespace Cosmodrone;

// @todo move to constants
const static auto FORWARD_VECTOR = tVec3f(0, 0, -1.f);
const static auto UP_VECTOR = tVec3f(0, 1.f, 0);
const static auto RIGHT_VECTOR = tVec3f(1.f, 0, 0);

const static float AUTO_DOCK_APPROACH_SPEED = 1500.f;
const static float AUTO_DOCK_APPROACH_SPEED_LIMIT = 2000.f;

// @todo move to engine
static inline float Lerpf(float a, float b, float alpha) {
  return a + (b - a) * alpha;
}

// @todo move to engine
static inline tVec3f Lerpf(const tVec3f& a, const tVec3f& b, const float alpha) {
  return tVec3f(
    Lerpf(a.x, b.x, alpha),
    Lerpf(a.y, b.y, alpha),
    Lerpf(a.z, b.z, alpha)
  );
}

// @todo remove in favor of LookRotation()
static Quaternion DirectionToQuaternion(const tVec3f& direction) {
  auto yaw = atan2f(direction.x, direction.z);
  auto pitch = atan2f(direction.xz().magnitude(), direction.y) - t_HALF_PI;

  return (
    Quaternion::fromAxisAngle(UP_VECTOR, yaw) *
    Quaternion::fromAxisAngle(RIGHT_VECTOR, pitch)
  );
}

static tVec3f GetDockingPositionOffset(const State& state) {
  if (state.docking_target.mesh_index == state.meshes.antenna_3) {
    return tVec3f(0, -1.f, -1.f).unit() * 0.7f;
  }

  return tVec3f(0, -1.f, -1.f).unit();
}

// @todo cleanup
void Autopilot::HandleAutopilot(Tachyon* tachyon, State& state, const float dt) {
  switch (state.flight_mode) {
    case FlightMode::AUTO_RETROGRADE: {
      // Figure out how 'backward' the ship is pointed
      float reverse_dot = tVec3f::dot(state.ship_rotation_basis.forward, state.ship_velocity_basis.forward);

      if (reverse_dot < -0.f) {
        // Use the current speed to determine how much we need to accelerate in the opposite direction
        float acceleration = state.ship_velocity.magnitude() / 2.f;
        if (acceleration > 5000.f) acceleration = 5000.f;
        if (acceleration < 500.f) acceleration = 500.f;

        // Increase acceleration the more the ship is aligned with the 'backward' vector
        float speed = acceleration * powf(-reverse_dot, 15.f);

        FlightSystem::ThrustForward(state, dt, speed);
      }

      if (state.ship_velocity.magnitude() < 200.f) {
        // Restore manual control when sufficiently decelerated
        state.flight_mode = FlightMode::MANUAL_CONTROL;
      }

      break;
    }

    case FlightMode::AUTO_DOCK: {
      if (state.auto_dock_stage == AutoDockStage::APPROACH_DECELERATION) {
        float backward_alignment = tVec3f::dot(state.ship_rotation_basis.forward, state.ship_velocity_basis.forward.invert());
        if (backward_alignment < 0.f) backward_alignment = 0.f;

        float deceleration_alpha = backward_alignment * backward_alignment * backward_alignment;
        float deceleration_factor = Lerpf(0.f, 2000.f, deceleration_alpha);

        state.ship_rotate_to_target_speed += 0.5f * dt;

        FlightSystem::ThrustForward(state, dt, deceleration_factor);

        if (state.ship_velocity.magnitude() < 50.f) {
          state.auto_dock_stage = AutoDockStage::APPROACH_ALIGNMENT;
          state.ship_rotate_to_target_speed = 0.f;
        }
      }

      if (
        state.auto_dock_stage == AutoDockStage::APPROACH &&
        state.ship_velocity.magnitude() < AUTO_DOCK_APPROACH_SPEED_LIMIT
      ) {
        FlightSystem::ThrustForward(state, dt, AUTO_DOCK_APPROACH_SPEED);
      }
    }
  }

  if (state.flight_mode == FlightMode::AUTO_PROGRADE) {
    state.target_ship_rotation = Quaternion::FromDirection(state.ship_velocity_basis.forward.invert(), state.ship_rotation_basis.up);
  }

  if (state.flight_mode == FlightMode::AUTO_RETROGRADE) {
    state.target_ship_rotation = Quaternion::FromDirection(state.ship_velocity_basis.forward, state.ship_rotation_basis.up);
  }

  if (state.flight_mode == FlightMode::AUTO_DOCK) {
    switch (state.auto_dock_stage) {
      case AutoDockStage::APPROACH_DECELERATION: {
        state.target_ship_rotation = DirectionToQuaternion(state.ship_velocity_basis.forward);

        break;
      }

      case AutoDockStage::APPROACH_ALIGNMENT: {
        auto docking_position = GetDockingPosition(tachyon, state);
        // @todo use live object
        auto target_object_rotation = state.docking_target.rotation;
        auto forward = (docking_position - state.ship_position).unit();
        auto target_object_up = target_object_rotation.getUpDirection();

        state.target_ship_rotation = Quaternion::FromDirection(forward.invert(), target_object_up);

        if (GetDockingAlignment(state, docking_position) > 0.99999f) {
          state.auto_dock_stage = AutoDockStage::APPROACH;
        } else {
          // @hack slow the ship down after deceleration
          state.ship_velocity *= (1.f - 5.f * dt);
        }

        break;
      }

      case AutoDockStage::APPROACH: {
        auto docking_position = GetDockingPosition(tachyon, state);
        auto docking_distance = (state.ship_position - docking_position).magnitude();

        if (state.ship_velocity.magnitude() >= AUTO_DOCK_APPROACH_SPEED_LIMIT && docking_distance < 10000.f) {
          state.auto_dock_stage = AutoDockStage::DOCKING;
        }

        break;
      }

      case AutoDockStage::DOCKING: {
        auto docking_position = GetDockingPosition(tachyon, state);
        auto docking_distance = (state.ship_position - docking_position).magnitude();

        state.target_ship_rotation =
          // @todo use live object
          state.docking_target.rotation *
          Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), t_PI);

        state.ship_rotate_to_target_speed = 0.3f;

        if (docking_distance < 5000.f) {
          float speed = AUTO_DOCK_APPROACH_SPEED_LIMIT * (docking_distance / 5000.f);

          state.ship_velocity = state.ship_velocity_basis.forward * speed;
        }

        // @todo @bug Ensure we never miss the target;
        // this seems to happen every so often!
        if (docking_distance < 300.f) {
          state.auto_dock_stage = AutoDockStage::DOCKED;
          state.ship_velocity = 0.f;
          state.ship_rotate_to_target_speed = 0.f;
          state.ship_camera_distance_target = 30000.f;

          state.target_camera_rotation =
            Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), 0.6f) *
            Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), t_PI * 1.2f) *
            state.docking_target.rotation.opposite();
        }
      }
    }
  }
}

bool Autopilot::IsAutopilotActive(const State& state) {
  return state.flight_mode != FlightMode::MANUAL_CONTROL;
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

float Autopilot::GetDockingAlignment(const State& state, const tVec3f& docking_position) {
  auto forward = state.ship_rotation_basis.forward;
  auto ship_to_target = (docking_position - state.ship_position).unit();

  return tVec3f::dot(forward, ship_to_target);
}