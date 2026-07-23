#include "engine/tachyon.h"

#include "metro/control_system.h"

using namespace metro;

const static auto GAMEPAD_X = tKey::CONTROLLER_A;
const static auto GAMEPAD_O = tKey::CONTROLLER_B;
const static auto GAMEPAD_SQUARE = tKey::CONTROLLER_X;
const static auto GAMEPAD_TRIANGLE = tKey::CONTROLLER_Y;

static bool DidPressPedalKey(Tachyon* tachyon) {
  if (did_press_key(GAMEPAD_X)) {
    return true;
  }

  // @todo keyboard support (?)

  return false;
}

static float GetSteering(Tachyon* tachyon) {
  float steering = tachyon->left_stick.x;

  return -1.f * steering;
}

// @todo move to utilities or elsewhere
static Bicycle* GetActiveBicycle(State& state) {
  for (auto& bike : state.bicycles) {
    if (bike.id == state.player_bike_id) {
      return &bike;
    }
  }

  return nullptr;
}

void ControlSystem::Update(Tachyon* tachyon, State& state) {
  auto* active_bike = GetActiveBicycle(state);

  if (active_bike == nullptr) {
    // @todo handle character controls
    return;
  }

  // Handle bicycle controls
  // @todo factor
  // @todo friction speed dampening
  {
    auto& bike = *active_bike;

    // Accelerating
    {
      if (DidPressPedalKey(tachyon)) {
        bike.speed += 50000.f * state.dt;
      }
    }

    // Steering
    {
      const float steering_speed = 4.f;
      const float rotation_speed = bike.speed / 1000.f;

      float steering = GetSteering(tachyon);

      bike.steering_angle = Tachyon_Lerpf(bike.steering_angle, steering, steering_speed * state.dt);

      float rotation_angle = bike.steering_angle * rotation_speed * state.dt;
      Quaternion rotation = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), rotation_angle);

      bike.facing_direction = rotation.toMatrix4f() * bike.facing_direction;
    }

    // Braking
    {
      const float braking_speed = 2.f;

      bike.speed *= 1.f - tachyon->right_trigger * braking_speed * state.dt;
    }

    // Wheels
    {
      const float wheel_revolution_speed = 0.001f;

      bike.wheel_revolution += bike.speed * wheel_revolution_speed * state.dt;
      bike.wheel_revolution = fmodf(bike.wheel_revolution, t_TAU);
    }

    bike.position += bike.facing_direction * bike.speed * state.dt;
  }
}