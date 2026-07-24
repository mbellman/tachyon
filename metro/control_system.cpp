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

static void HandleBikeControls(Tachyon* tachyon, State& state, Bicycle& bike) {
  // @todo define per-bicycle
  const float pedal_impulse = 300000.f;
  const float top_speed = 30000.f;

  // Pedaling
  // @todo rock the bike a little bit
  {
    if (DidPressPedalKey(tachyon)) {
      bike.pedal_speed += pedal_impulse * state.dt;
    }

    // Dampen pedal speed
    bike.pedal_speed *= 1.f - state.dt;

    // Increase bike speed as pedals rotate
    bike.speed += bike.pedal_speed * 0.8f * state.dt;

    // Revolve pedals in proportion to speed
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
    const float steering_speed = Tachyon_Lerpf(2.f, 0.1f, sqrtf(speed_ratio));
    const float rotation_speed = bike.speed / 1200.f;

    float target_steering_angle = 1.2f * GetSteering(tachyon);

    // Apply steering
    bike.steering_angle = Tachyon_Lerpf(
      bike.steering_angle,
      target_steering_angle,
      steering_speed * state.dt
    );

    // Reduce steering with speed
    bike.steering_angle *= 1.f - (bike.speed / 10000.f) * state.dt;

    // Calculate turning radius r = w / δ * cos(φ)
    float w = 2400.f;
    float delta = bike.steering_angle;
    float phi = t_HALF_PI * 0.1777f;
    float radius = w / (delta * cosf(phi));

    // Calculate instantaneous turn angle
    float turn_angle;

    if (std::isinf(radius)) {
      // If the radius is infinite, don't turn at all
      turn_angle = 0.f;
    } else {
      // Use bike.speed * state.dt as an approximation of arc length L.
      // The turn angle is just computed as L / radius.
      turn_angle = (bike.speed * state.dt) / radius;
    }

    // Turn and update the facing direction
    Quaternion turn_rotation = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), turn_angle);

    bike.facing_direction = turn_rotation.toMatrix4f() * bike.facing_direction;
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

void ControlSystem::Update(Tachyon* tachyon, State& state) {
  profile("ControlSystem::Update()");

  auto* active_bike = GetActiveBicycle(state);

  if (active_bike == nullptr) {
    // @todo handle character controls
  } else {
    HandleBikeControls(tachyon, state, *active_bike);
  }
}