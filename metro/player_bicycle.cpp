#include "engine/tachyon.h"

#include "metro/player_bicycle.h"
#include "metro/bikes/common_bike.h"
#include "metro/utilities.h"

using namespace metro;

const static int PHYSICS_ITERATIONS = 3;

static void ShowDebugVisuals(Tachyon* tachyon, State& state, const Bicycle& bike) {
  tVec3f steering_direction =
    Quaternion::fromAxisAngle(AXIS_Y, bike.steering_angle).toMatrix4f() *
    bike.facing_direction;

  tVec3f steering_position = bike.front_wheel_position + tVec3f(0, 500.f, 0);
  tVec3f steering_vector = steering_direction * 3000.f;

  tVec3f momentum_position = bike.front_wheel_position + tVec3f(0, 250.f, 0);
  tVec3f momentum_vector = bike.momentum * 0.01f;

  Debug::ShowDebugSphere(tachyon, bike.position, 300.f);
  Debug::ShowDebugSphere(tachyon, bike.front_wheel_position, 150.f);
  Debug::ShowDebugSphere(tachyon, bike.back_wheel_position, 150.f);

  Debug::ShowDebugVector(tachyon, bike.front_wheel_position, bike.movement_vector * 2000.f, tVec3f(1.f, 0, 1.f));
  Debug::ShowDebugVector(tachyon, steering_position, steering_vector, tVec3f(0, 0, 1.f));
  Debug::ShowDebugVector(tachyon, momentum_position, momentum_vector, tVec3f(0, 1.f, 0));

  Debug::ShowDebugVector(tachyon, bike.front_wheel_position, bike.front_wheel_slope * 1000.f, tVec3f(1.f, 0, 0));
  Debug::ShowDebugVector(tachyon, bike.back_wheel_position, bike.back_wheel_slope * 1000.f, tVec3f(1.f, 0, 0));
}

static void UpdateBikePositionInFreefall(Bicycle& bike, const float dt) {
  bike.position += (bike.momentum / 25.f) * dt;
}

static void UpdateBikePositionOnGround(Bicycle& bike, const float dt) {
  tVec3f movement_direction = GetMovementDirection(bike);
  tVec3f average_wheel_slope = (bike.front_wheel_slope + bike.back_wheel_slope) / 2.f;
  tVec3f target_movement_vector = bike.facing_direction;

  // Moving downhill; apply gravity along the direction of the average wheel slope,
  // forcing a downward bias and keeping the bike wheels locked to the ground
  if (movement_direction.y < -0.1f) {
    target_movement_vector -= average_wheel_slope;
    target_movement_vector = target_movement_vector.unit();
  }

  // Moving backward downhill; similar to above except our movement vector is inverted,
  // since the bike speed directs us negatively along the vector. Thus, we apply the
  // inverted average wheel slope here.
  if (bike.speed < 0.f) {
    target_movement_vector -= average_wheel_slope.invert();
    target_movement_vector = target_movement_vector.unit();
  }

  float movement_vector_blend_rate = Tachyon_Lerpf(1.f, 3.f * dt, bike.drifting_factor);

  bike.movement_vector = tVec3f::lerp(
    bike.movement_vector,
    target_movement_vector,
    movement_vector_blend_rate
  ).unit();

  bike.position += bike.movement_vector * bike.speed * dt;
}

void PlayerBicycle::Update(Tachyon* tachyon, State& state) {
  profile("PlayerBicycle::Update()");

  auto* active_bike = GetActiveBicycle(state);

  if (active_bike == nullptr) {
    return;
  }

  // Position update
  {
    auto& bike = *active_bike;

    if (bike.in_freefall || bike.jumping_off_ramp) {
      UpdateBikePositionInFreefall(bike, state.dt);
    } else {
      UpdateBikePositionOnGround(bike, state.dt);
    }

    CommonBike::Update(tachyon, state, bike, state.player_bike_index);
  }

  // Physics
  {
    state.dt /= (float) PHYSICS_ITERATIONS;

    for_range(1, PHYSICS_ITERATIONS) {
      switch (active_bike->type) {
        case COMMON_BIKE:
          CommonBike::HandlePhysics(tachyon, state, *active_bike);
          CommonBike::Update(tachyon, state, *active_bike, state.player_bike_index);
          break;
        default:
          break;
      }
    }

    state.dt *= (float) PHYSICS_ITERATIONS;
  }

  // @todo dev mode only
  if (tachyon->show_timing_profile) {
    auto& bike = *active_bike;

    ShowDebugVisuals(tachyon, state, bike);
  }
}