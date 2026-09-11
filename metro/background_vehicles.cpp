#include "engine/tachyon.h"

#include "metro/background_vehicles.h"
#include "metro/vehicles/common_bike.h"
#include "metro/vehicles/electric_scooter.h"

using namespace metro;

void BackgroundVehicles::Update(Tachyon* tachyon, State& state) {
  profile("BackgroundVehicles::Update()");

  int32 total_common_bikes = 0;

  for (auto& bike : state.bicycles) {
    switch (bike.type) {
      case COMMON_BIKE:
        CommonBike::Update(tachyon, state, bike, total_common_bikes++);
        break;
      default:
        break;
    }
  }

  for (auto& scooter : state.scooters) {
    switch (scooter.type) {
      case ELECTRIC_SCOOTER:
        // @todo
        break;
      default:
        break;
    }
  }
}

void BackgroundVehicles::SpawnBicycle(Tachyon* tachyon, State& state, Bicycle& bike) {
  // Precompute rotation
  bike.flat_rotation = Quaternion::FromDirection(bike.facing_direction, Y_UP);

  // Initialize previous position
  bike.previous_position = bike.position;

  switch (bike.type) {
    case COMMON_BIKE:
      CommonBike::Spawn(tachyon, state, bike);
      break;
    default:
      break;
  }

  state.bicycles.push_back(bike);
}

void BackgroundVehicles::SpawnScooter(Tachyon* tachyon, State& state, Scooter& scooter) {
  switch (scooter.type) {
    case ELECTRIC_SCOOTER:
      ElectricScooter::Spawn(tachyon, state, scooter);
      break;
    default:
      break;
  }

  state.scooters.push_back(scooter);
}

void BackgroundVehicles::DestroyBicycle(Tachyon* tachyon, State& state, Bicycle& bike) {
  switch (bike.type) {
    case COMMON_BIKE:
      CommonBike::Destroy(tachyon, state, bike);
      break;
    default:
      break;
  }

  for_reversed(state.bicycles) {
    if (bike.id == state.bicycles[i].id) {
      state.bicycles.erase(state.bicycles.begin() + i);

      break;
    }
  }
}