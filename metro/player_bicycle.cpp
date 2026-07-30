#include "engine/tachyon.h"

#include "metro/player_bicycle.h"
#include "metro/bikes/common_bike.h"
#include "metro/constants.h"
#include "metro/debug.h"
#include "metro/utilities.h"

using namespace metro;

const static int PHYSICS_ITERATIONS = 3;

static void ShowDebugVisuals(Tachyon* tachyon, State& state, const Bicycle& bike) {
  tVec3f movement_direction = GetMovementDirection(bike);

  tVec3f steering_direction =
    Quaternion::fromAxisAngle(AXIS_Y, bike.steering_angle).toMatrix4f() *
    bike.facing_direction;

  tVec3f steering_position = bike.front_wheel_position + tVec3f(0, 500.f, 0);
  tVec3f steering_vector = steering_direction * 3000.f;

  tVec3f momentum_position = bike.front_wheel_position + tVec3f(0, 250.f, 0);
  tVec3f momentum_vector = bike.momentum * 0.01f;

  Debug::ShowDebugSphere(tachyon, state, bike.position, 300.f);
  Debug::ShowDebugSphere(tachyon, state, bike.front_wheel_position, 150.f);
  Debug::ShowDebugSphere(tachyon, state, bike.back_wheel_position, 150.f);

  Debug::ShowDebugVector(tachyon, state, bike.front_wheel_position, movement_direction * 2000.f, tVec3f(1.f, 0, 1.f));
  Debug::ShowDebugVector(tachyon, state, steering_position, steering_vector, tVec3f(0, 0, 1.f));
  Debug::ShowDebugVector(tachyon, state, momentum_position, momentum_vector, tVec3f(0, 1.f, 0));
}

void PlayerBicycle::Update(Tachyon* tachyon, State& state) {
  profile("PlayerBicycle::Update()");

  auto* active_bike = GetActiveBicycle(state);

  if (active_bike == nullptr) {
    return;
  }

  // Motion + position update
  {
    auto& bike = *active_bike;

    if (bike.in_freefall) {
      bike.position += (bike.momentum / 25.f) * state.dt;
    } else {
      tVec3f movement_direction = GetMovementDirection(bike);
      tVec3f target_movement_vector;

      // @todo fix the direction of the velocity applied here;
      // we should be applying the correct directional force
      // based on the ground slope across both wheels
      if (movement_direction.y < 0.f || bike.speed < 0.f) {
        target_movement_vector = movement_direction;
      } else {
        target_movement_vector = bike.facing_direction;
      }

      float movement_vector_blend_rate = Tachyon_Lerpf(1.f, 3.f * state.dt, bike.drifting_factor);

      bike.movement_vector = tVec3f::lerp(
        bike.movement_vector,
        target_movement_vector,
        movement_vector_blend_rate
      ).unit();

      bike.position += bike.movement_vector * bike.speed * state.dt;
    }

    CommonBike::Update(tachyon, state, bike, state.player_bike_index);
  }

  // Physics
  {
    state.dt /= (float) PHYSICS_ITERATIONS;

    for_range(1, PHYSICS_ITERATIONS) {
      switch (active_bike->type) {
        case BicycleType::COMMON_BIKE:
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