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
  profile("ControlSystem::Update()");

  auto* active_bike = GetActiveBicycle(state);

  if (active_bike == nullptr) {
    // @todo handle character controls
    return;
  }

  // Handle bicycle controls
  // @todo factor
  // @todo more physically grounded steering/turning
  {
    auto& bike = *active_bike;

    // @todo define per-bicycle
    const float pedal_impulse = 300000.f;
    const float acceleration_impulse = 100000.f;
    const float top_speed = 30000.f;

    // Pedaling
    // @todo smoothly increase velocity
    // @todo rock the bike a little bit
    {
      if (DidPressPedalKey(tachyon)) {
        bike.pedal_speed += pedal_impulse * state.dt;
        bike.speed += acceleration_impulse * state.dt;
      }

      bike.pedal_speed *= 1.f - state.dt;
      bike.pedal_revolution += bike.pedal_speed * 0.001f * state.dt;
      bike.pedal_revolution = fmodf(bike.pedal_revolution, t_TAU);
    }

    // Speed dampening
    {
      float speed_ratio = bike.speed / top_speed;
      float friction = 0.025f + 0.4f * powf(speed_ratio, 20.f);

      bike.speed *= 1.f - friction * state.dt;
    }

    // Steering
    {
      float speed_ratio = bike.speed / top_speed;
      const float steering_speed = Tachyon_Lerpf(2.f, 0.f, sqrtf(speed_ratio));
      const float rotation_speed = bike.speed / 1200.f;

      float steering = GetSteering(tachyon);

      // Apply steering
      bike.steering_angle = Tachyon_Lerpf(bike.steering_angle, steering, steering_speed * state.dt);

      // Reduce steering with speed
      bike.steering_angle *= 1.f - (bike.speed / 10000.f) * state.dt;

      float rotation_angle = bike.steering_angle * rotation_speed * state.dt;
      Quaternion rotation = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), rotation_angle);

      bike.facing_direction = rotation.toMatrix4f() * bike.facing_direction;
    }

    // Leaning
    {
      float speed_ratio = bike.speed / top_speed;

      float steering = GetSteering(tachyon);
      float target_angle = 0.6f * sqrtf(speed_ratio) * -steering;
      float blend_speed = is_moving_left_stick() ? 2.f : 4.f;

      bike.leaning_angle = Tachyon_Lerpf(bike.leaning_angle, target_angle, blend_speed * state.dt);
    }

    // Braking
    {
      const float braking_speed = 2.f;

      // Left brake
      bike.speed *= 1.f - tachyon->left_trigger * braking_speed * state.dt;

      // Right brake
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