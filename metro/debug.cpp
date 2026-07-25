#include "engine/tachyon.h"

#include "metro/debug.h"

using namespace metro;

void Debug::HandleFrameStart(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  reset_instances(meshes.debug_ring);
  reset_instances(meshes.debug_sphere);
  reset_instances(meshes.debug_line);
  reset_instances(meshes.debug_cone);
}

void Debug::ShowDebugSphere(Tachyon* tachyon, State& state, const tVec3f& position, const float radius) {
  auto& sphere = use_instance(state.meshes.debug_sphere);

  sphere.position = position;
  sphere.scale = tVec3f(radius);
  sphere.color = 0x00F8;

  commit(sphere);
}

void Debug::ShowDebugVector(Tachyon* tachyon, State& state, const tVec3f& position, const tVec3f& vector, const tVec3f& color) {
  float length = vector.magnitude();
  tVec3f direction = vector / length;

  auto& line = use_instance(state.meshes.debug_line);

  tVec3f up = vector.y != 0.f && vector.x == 0.f && vector.z == 0.f
    ? tVec3f(1.f, 0, 0)
    : tVec3f(0, 1.f, 0);

  line.position = position;
  line.rotation = Quaternion::FromDirection(direction, up);
  line.scale.y = 150.f;
  line.scale.x = 150.f;
  line.scale.z = length;
  line.color = tVec4f(color, 0.5f);

  commit(line);

  auto& cone = use_instance(state.meshes.debug_cone);

  cone.position = position + vector;
  cone.rotation = line.rotation;
  cone.scale = tVec3f(400.f);
  cone.color = line.color;

  commit(cone);
}