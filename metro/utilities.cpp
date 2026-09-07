#include "metro/utilities.h"

using namespace metro;

tVec3f metro::UnitBikeToWorldPosition(const Bicycle& bike, const tVec3f& position) {
  tVec3f translation = bike.position;
  Quaternion rotation = bike.flat_rotation;
  tVec3f scale = tVec3f(2000.f);

  return translation + rotation.toMatrix4f() * (position * scale);
}

tVec3f metro::UnitVisualBikeToWorldPosition(const Bicycle& bike, const tVec3f& position) {
  tVec3f translation = bike.visual_position;
  Quaternion rotation = bike.visual_rotation;
  tVec3f scale = tVec3f(2000.f);

  return translation + rotation.toMatrix4f() * (position * scale);
}

tVec3f metro::UnitObjectToWorldPosition(const tObject object, const tVec3f& position) {
  tVec3f translation = object.position;
  Quaternion rotation = object.rotation;
  tVec3f scale = object.scale;

  return translation + rotation.toMatrix4f() * (position * scale);
}

tVec3f metro::GetMovementDirection(const Bicycle& bike) {
  return bike.directional_rotation.getDirection().invert();
}

BaseVehicle* metro::GetActiveVehicle(State& state) {
  for (auto& bike : state.bicycles) {
    if (bike.id == state.player_bike_id) {
      return &bike;
    }
  }

  return nullptr;
}

EntityCategory metro::GetVehicleCategory(BaseVehicle* vehicle) {
  if (vehicle == nullptr) {
    return NOT_AN_ENTITY;
  }

  switch (vehicle->type) {
    case COMMON_BIKE:
      return EntityCategory::BICYCLE;
    case ELECTRIC_SCOOTER:
      return EntityCategory::SCOOTER;
    default:
      return NOT_AN_ENTITY;
  }
}