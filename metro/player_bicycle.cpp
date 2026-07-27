#include "engine/tachyon.h"

#include "metro/player_bicycle.h"
#include "metro/bikes/common_bike.h"
#include "metro/debug.h"
#include "metro/utilities.h"

using namespace metro;

void PlayerBicycle::Update(Tachyon* tachyon, State& state) {
  profile("PlayerBicycle::Update()");

  auto* active_bike = GetActiveBicycle(state);

  if (active_bike == nullptr) {
    return;
  }

  switch (active_bike->type) {
    case BicycleType::COMMON_BIKE:
      CommonBike::HandlePhysics(tachyon, state, *active_bike);
      CommonBike::Update(tachyon, state, *active_bike, state.player_bike_index);
      break;
    default:
      break;
  }

  // @todo dev mode only
  if (tachyon->show_timing_profile) {
    auto& bike = *active_bike;

    tVec3f steering_direction =
      Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), bike.steering_angle).toMatrix4f() *
      bike.facing_direction;

    tVec3f steering_vector = steering_direction * 3000.f;
    tVec3f momentum_vector = bike.momentum * 0.01f;

    Debug::ShowDebugSphere(tachyon, state, bike.front_wheel_position, 150.f);
    Debug::ShowDebugSphere(tachyon, state, bike.back_wheel_position, 150.f);

    Debug::ShowDebugVector(tachyon, state, bike.front_wheel_position, steering_vector, tVec3f(0, 0, 1.f));
    Debug::ShowDebugVector(tachyon, state, bike.front_wheel_position, momentum_vector, tVec3f(0, 1.f, 0));
  }
}