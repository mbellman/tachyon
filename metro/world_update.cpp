#include "engine/tachyon.h"

#include "metro/world_update.h"

using namespace metro;

// @temporary
static void UpdateCommonBike(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  auto& frame = objects(meshes.common_frame)[0];
  auto& skeleton = objects(meshes.common_skeleton)[0];
  auto& handles = objects(meshes.common_handles)[0];
  auto& seat = objects(meshes.common_seat)[0];
  auto& wheels = objects(meshes.common_wheels)[0];

  Quaternion rotation = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), get_scene_time());

  frame.rotation = rotation;
  skeleton.rotation = rotation;
  handles.rotation = rotation;
  seat.rotation = rotation;
  wheels.rotation = rotation;

  frame.color = tVec3f(0.5f, 1.f, 0.4f);
  frame.material = tVec4f(0.3f, 0, 0.2f, 0);

  skeleton.color = tVec3f(0.8f);
  skeleton.material = tVec4f(0.4f, 1.f, 0, 0);

  handles.color = tVec3f(0.1f);
  handles.material = tVec4f(0.7f, 0, 0, 0.5f);

  seat.color = tVec3f(0.1f, 0, 0);
  seat.material = tVec4f(0.6f, 0, 0, 0.2f);

  wheels.color = tVec3f(1.f, 0.9f, 0.7f);
  wheels.material = tVec4f(0.9f, 0, 0, 0.5f);

  commit(frame);
  commit(skeleton);
  commit(handles);
  commit(seat);
  commit(wheels);
}

void World::Update(Tachyon* tachyon, State& state) {
  // @temporary
  tachyon->scene.primary_light_direction = tVec3f(0.5f, -1.f, 0.2f);

  // @temporary
  UpdateCommonBike(tachyon, state);
}