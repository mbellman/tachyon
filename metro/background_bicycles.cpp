#include "engine/tachyon.h"
#include "metro/background_bicycles.h"

using namespace metro;

static tVec3f UnitBikeToWorldPosition(const Bicycle& bicycle, const tVec3f& position) {
  tVec3f translation = bicycle.position;
  Quaternion rotation = bicycle.rotation;
  tVec3f scale = tVec3f(2000.f);

  return translation + rotation.toMatrix4f() * (position * scale);
}

static void SpawnCommonBike(Tachyon* tachyon, State& state, const Bicycle& bicycle) {
  auto& meshes = state.meshes;

  auto& frame = create(meshes.common_frame);
  auto& skeleton = create(meshes.common_chassis);
  auto& handles = create(meshes.common_handles);
  auto& seat = create(meshes.common_seat);
  auto& wheel1 = create(meshes.common_wheel);
  auto& spokes1 = create(meshes.common_spokes);
  auto& wheel2 = create(meshes.common_wheel);
  auto& spokes2 = create(meshes.common_spokes);

  frame.scale = tVec3f(2000.f);
  skeleton.scale = tVec3f(2000.f);
  handles.scale = tVec3f(2000.f);
  seat.scale = tVec3f(2000.f);
  wheel1.scale = tVec3f(2000.f);
  spokes1.scale = tVec3f(2000.f);
  wheel2.scale = tVec3f(2000.f);
  spokes2.scale = tVec3f(2000.f);

  commit(frame);
  commit(skeleton);
  commit(handles);
  commit(seat);
  commit(wheel1);
  commit(spokes1);
  commit(wheel2);
  commit(spokes2);
}

static void UpdateCommonBike(Tachyon* tachyon, State& state, const Bicycle& bicycle, int32 index) {
  auto& meshes = state.meshes;

  auto& frame = objects(meshes.common_frame)[index];
  auto& skeleton = objects(meshes.common_chassis)[index];
  auto& handles = objects(meshes.common_handles)[index];
  auto& seat = objects(meshes.common_seat)[index];

  frame.position = bicycle.position;
  frame.rotation = bicycle.rotation;
  frame.color = bicycle.frame_color;
  frame.material = tVec4f(0.3f, 0, 0.2f, 0);

  skeleton.position = bicycle.position;
  skeleton.rotation = bicycle.rotation;
  skeleton.color = tVec3f(0.8f);
  skeleton.material = tVec4f(0.4f, 1.f, 0, 0);

  handles.position = bicycle.position;
  handles.rotation = bicycle.rotation;
  handles.color = bicycle.handles_color;
  handles.material = tVec4f(0.7f, 0, 0, 0.5f);

  seat.position = bicycle.position;
  seat.rotation = bicycle.rotation;
  seat.color = bicycle.seat_color;
  seat.material = tVec4f(0.6f, 0, 0, 0.2f);

  commit(frame);
  commit(skeleton);
  commit(handles);
  commit(seat);

  // Wheels
  {
    int32 wheel_index = index * 2;

    auto& front_wheel = objects(meshes.common_wheel)[wheel_index];
    auto& front_spokes = objects(meshes.common_spokes)[wheel_index];

    auto& back_wheel = objects(meshes.common_wheel)[wheel_index + 1];
    auto& back_spokes = objects(meshes.common_spokes)[wheel_index + 1];

    // @temporary
    front_wheel.position = UnitBikeToWorldPosition(bicycle, tVec3f(0.59f, 0, 0));
    front_wheel.rotation = bicycle.rotation;
    front_wheel.color = bicycle.wheel_color;
    front_wheel.material = tVec4f(0.9f, 0, 0, 0.5f);

    front_spokes.position = front_wheel.position;
    front_spokes.rotation = bicycle.rotation;
    front_spokes.color = tVec3f(0.8f);
    front_spokes.material = tVec4f(0.4f, 1.f, 0, 0);

    // @temporary
    back_wheel.position = UnitBikeToWorldPosition(bicycle, tVec3f(-0.61f, 0, 0));
    back_wheel.rotation = bicycle.rotation;
    back_wheel.color = bicycle.wheel_color;
    back_wheel.material = tVec4f(0.9f, 0, 0, 0.5f);

    back_spokes.position = back_wheel.position;
    back_spokes.rotation = bicycle.rotation;
    back_spokes.color = tVec3f(0.8f);
    back_spokes.material = tVec4f(0.4f, 1.f, 0, 0);

    commit(front_wheel);
    commit(front_spokes);
    commit(back_wheel);
    commit(back_spokes);
  }
}

void BackgroundBicycles::Update(Tachyon* tachyon, State& state) {
  int32 total_common_bikes = 0;

  for (auto& bicycle : state.bicycles) {
    // @temporary
    bicycle.rotation = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), 0.5f * get_scene_time());

    switch (bicycle.type) {
      case BicycleType::COMMON_BIKE:
        UpdateCommonBike(tachyon, state, bicycle, total_common_bikes++);
        break;
      default:
        break;
    }
  }
}

void BackgroundBicycles::SpawnBicycle(Tachyon* tachyon, State& state, const Bicycle& bicycle) {
  switch (bicycle.type) {
    case BicycleType::COMMON_BIKE:
      SpawnCommonBike(tachyon, state, bicycle);
      break;
    default:
      break;
  }

  state.bicycles.push_back(bicycle);
}