#include "engine/tachyon.h"
#include "metro/background_bicycles.h"

using namespace metro;

static void SpawnCommonBike(Tachyon* tachyon, State& state, const Bicycle& bicycle) {
  auto& meshes = state.meshes;

  auto& frame = create(meshes.common_frame);
  auto& skeleton = create(meshes.common_skeleton);
  auto& handles = create(meshes.common_handles);
  auto& seat = create(meshes.common_seat);
  auto& wheels = create(meshes.common_wheels);

  frame.scale = tVec3f(2000.f);
  skeleton.scale = tVec3f(2000.f);
  handles.scale = tVec3f(2000.f);
  seat.scale = tVec3f(2000.f);
  wheels.scale = tVec3f(2000.f);

  commit(frame);
  commit(skeleton);
  commit(handles);
  commit(seat);
  commit(wheels);
}

static void UpdateCommonBike(Tachyon* tachyon, State& state, const Bicycle& bicycle, int32 index) {
  auto& meshes = state.meshes;

  auto& frame = objects(meshes.common_frame)[index];
  auto& skeleton = objects(meshes.common_skeleton)[index];
  auto& handles = objects(meshes.common_handles)[index];
  auto& seat = objects(meshes.common_seat)[index];
  auto& wheels = objects(meshes.common_wheels)[index];

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

  wheels.position = bicycle.position;
  wheels.rotation = bicycle.rotation;
  wheels.color = bicycle.wheel_color;
  wheels.material = tVec4f(0.9f, 0, 0, 0.5f);

  commit(frame);
  commit(skeleton);
  commit(handles);
  commit(seat);
  commit(wheels);
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