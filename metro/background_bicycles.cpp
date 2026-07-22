#include "engine/tachyon.h"

#include "metro/background_bicycles.h"
#include "metro/bikes/common_bike.h"

using namespace metro;

void BackgroundBicycles::Update(Tachyon* tachyon, State& state) {
  int32 total_common_bikes = 0;

  for (auto& bicycle : state.bicycles) {
    bicycle.facing_direction.x = sinf(get_scene_time());
    bicycle.facing_direction.z = cosf(get_scene_time());

    bicycle.wheel_revolution += 5.f * state.dt;
    bicycle.wheel_revolution = fmodf(bicycle.wheel_revolution, t_TAU);
    bicycle.steering_angle = sinf(get_scene_time());
    bicycle.leaning_angle = 0.5f * cosf(get_scene_time() + 0.5f);

    switch (bicycle.type) {
      case BicycleType::COMMON_BIKE:
        CommonBike::Update(tachyon, state, bicycle, total_common_bikes++);
        break;
      default:
        break;
    }
  }
}

void BackgroundBicycles::SpawnBicycle(Tachyon* tachyon, State& state, const Bicycle& bike) {
  switch (bike.type) {
    case BicycleType::COMMON_BIKE:
      CommonBike::Spawn(tachyon, state, bike);
      break;
    default:
      break;
  }

  state.bicycles.push_back(bike);
}