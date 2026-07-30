#include "engine/tachyon.h"

#include "metro/control_system.h"
#include "metro/constants.h"
#include "metro/debug.h"
#include "metro/utilities.h"

using namespace metro;

// @todo move to Debug::
static void DebugShowRadiusRing(Tachyon* tachyon, State& state, const Bicycle& bike, const float radius) {
  auto& ring = use_instance(state.meshes.debug_ring);
  float absolute_radius = abs(radius);

  if (absolute_radius < 20000.f) {
    tVec3f offset = tVec3f::cross(tVec3f(0, 1.f, 0), bike.facing_direction);
    tVec3f pivot = UnitBikeToWorldPosition(bike, tVec3f(0, 0, 0.61f));

    ring.position = bike.position + offset * radius;
    ring.position.y -= 725.f;
    ring.scale = tVec3f(absolute_radius);
  } else {
    ring.scale = tVec3f(0.f);
  }

  commit(ring);
}

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

static void HandleCharacterControls(Tachyon* tachyon, State& state) {
  auto& camera = tachyon->scene.camera;

  // Acceleration/movement
  {
    float acceleration = is_key_held(GAMEPAD_X) ? 14000.f : 8000.f;
    tVec3f ground_forward = camera.orientation.getDirection().xz().unit();
    tVec3f ground_left = tVec3f::cross(Y_UP, ground_forward);

    state.player_velocity += ground_forward * -tachyon->left_stick.y * acceleration;
    state.player_velocity += ground_left * -tachyon->left_stick.x * acceleration;
    state.recorded_player_speed = state.player_velocity.magnitude();

    // Top speed dampening
    if (state.recorded_player_speed > acceleration) {
      // @todo this doesn't properly limit us to a defined top speed
      state.player_velocity *= 1.f - 35.f * state.dt;
    }

    state.player_position += state.player_velocity * state.dt;

    // Velocity falloff/friction
    state.player_velocity *= 1.f - 5.f * state.dt;
    state.recorded_player_speed = state.player_velocity.magnitude();

    // Stop at low velocities
    if (state.recorded_player_speed < 100.f) {
      state.player_velocity = tVec3f(0.f);
      state.recorded_player_speed = 0.f;
    }
  }

  // Determine camera auto-centering behavior
  {
    if (is_moving_left_stick() && !is_moving_right_stick()) {
      state.target_camera_azimuth = atan2f(state.player_velocity.z, state.player_velocity.x) + t_PI;

      state.target_camera_azimuth_blend_rate = Tachyon_Lerpf(
        state.target_camera_azimuth_blend_rate,
        1.f,
        state.dt
      );
    } else {
      state.target_camera_azimuth_blend_rate = Tachyon_Lerpf(
        state.target_camera_azimuth_blend_rate,
        0.f,
        state.dt
      );
    }
  }

  // Interactions
  {
    if (did_press_key(GAMEPAD_TRIANGLE)) {
      for (auto& bike : state.bicycles) {
        float distance = tVec3f::distance(bike.position, state.player_position);

        if (distance < 2000.f) {
          state.player_bike_id = bike.id;

          break;
        }
      }
    }
  }
}

static void HandleBikeControls(Tachyon* tachyon, State& state, Bicycle& bike) {
  // @todo define per-bicycle
  const float pedal_impulse = 300000.f;
  const float top_speed = 30000.f;

  // Pedaling
  {
    if (DidPressPedalKey(tachyon)) {
      bike.pedal_speed += pedal_impulse * state.dt;
    }

    // Dampen pedal speed
    bike.pedal_speed *= 1.f - state.dt;

    // Increase bike speed as pedals rotate
    bike.speed += bike.pedal_speed * 0.8f * state.dt;

    // Revolve pedals in proportion to speed
    bike.pedal_revolution += bike.pedal_speed * 0.0005f * state.dt;
    bike.pedal_revolution = fmodf(bike.pedal_revolution, t_TAU);
  }

  // Rock back and forth when pedaling
  {
    // Use the pedal speed to determine the intensity of the rocking
    float pedal_factor = bike.pedal_speed / 20000.f;
    if (pedal_factor > 1.f) pedal_factor = 1.f;

    // Diminish the effect at lower speeds
    pedal_factor *= pedal_factor;
    pedal_factor *= pedal_factor;

    float target_rocking_factor = 0.25f * pedal_factor * sinf(bike.pedal_revolution);

    bike.rocking_factor = Tachyon_Lerpf(
      bike.rocking_factor,
      target_rocking_factor,
      4.f * state.dt
    );
  }

  // Speed dampening
  {
    float speed_ratio = abs(bike.speed) / top_speed;
    float friction = 0.025f + 0.4f * powf(speed_ratio, 20.f);

    bike.speed *= 1.f - friction * state.dt;
  }

  // Steering
  {
    float absolute_speed = abs(bike.speed);
    float speed_ratio = absolute_speed / top_speed;
    float steering_speed = Tachyon_Lerpf(2.f, 0.1f, sqrtf(speed_ratio));

    float target_steering_angle = 1.2f * GetSteering(tachyon);

    // Apply steering
    bike.steering_angle = Tachyon_Lerpf(
      bike.steering_angle,
      target_steering_angle,
      steering_speed * state.dt
    );

    // Reduce steering with speed
    bike.steering_angle *= 1.f - (absolute_speed / 10000.f) * state.dt;

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
      // Use bike.speed * dt as an approximation of arc length L.
      // The turn angle is just computed as L / radius.
      turn_angle = (bike.speed * state.dt) / radius;
    }

    // Oversteer when drifting
    if (bike.drifting) {
      turn_angle *= 3.f;
    }

    // Turn and update the facing direction
    Quaternion turn_rotation = Quaternion::fromAxisAngle(AXIS_Y, turn_angle);

    bike.facing_direction = turn_rotation.toMatrix4f() * bike.facing_direction;
    bike.facing_direction = bike.facing_direction.unit();

    // @todo dev mode only
    if (tachyon->show_timing_profile) {
      DebugShowRadiusRing(tachyon, state, bike, radius);
    }
  }

  // Leaning
  {
    float speed_ratio = abs(bike.speed) / top_speed;

    float steering = GetSteering(tachyon);
    float target_angle = 0.6f * sqrtf(speed_ratio) * -steering;
    float blend_speed = is_moving_left_stick() ? 2.f : 4.f;

    bike.leaning_angle = Tachyon_Lerpf(bike.leaning_angle, target_angle, blend_speed * state.dt);
  }

  // Drifting
  {
    bike.drifting = (
      tachyon->left_trigger == 0.f &&
      bike.speed > 20000.f && (
        (tachyon->left_stick.x > 0.5f && tachyon->right_trigger > 0.5f) ||
        (tachyon->left_stick.x < -0.5f && tachyon->right_trigger > 0.5f)
      )
    );

    if (bike.drifting) {
      // Transition into drifting
      bike.drifting_factor = Tachyon_Lerpf(bike.drifting_factor, 1.f, 2.f * state.dt);
    } else {
      // Transition out of drifting
      bike.drifting_factor = Tachyon_Lerpf(bike.drifting_factor, 0., state.dt);
    }
  }

  // Braking
  if (!bike.drifting) {
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

  // Getting off the bike
  {
    if (
      did_press_key(GAMEPAD_TRIANGLE) &&
      !bike.in_freefall
    ) {
      state.player_bike_id = -1;
      bike.pedal_speed = 0.f;
      bike.speed = 0.f;
    }
  }
}

void ControlSystem::Update(Tachyon* tachyon, State& state) {
  profile("ControlSystem::Update()");

  auto* active_bike = GetActiveBicycle(state);

  if (active_bike == nullptr) {
    HandleCharacterControls(tachyon, state);
  } else {
    HandleBikeControls(tachyon, state, *active_bike);
  }
}