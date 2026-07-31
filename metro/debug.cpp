#include "engine/tachyon.h"

#include "metro/debug.h"

using namespace metro;

void Debug::Reset(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  reset_instances(meshes.debug_ring);
  reset_instances(meshes.debug_sphere);
  reset_instances(meshes.debug_plane);
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

void Debug::ShowDebugLine(Tachyon* tachyon, State& state, const DebugLineConfig& config) {
  auto& line = use_instance(state.meshes.debug_line);

  const tVec3f& vector = config.vector;
  float length = vector.magnitude();
  tVec3f direction = vector / length;

  tVec3f up = vector.y != 0.f && vector.x == 0.f && vector.z == 0.f
    ? tVec3f(1.f, 0, 0)
    : tVec3f(0, 1.f, 0);

  line.position = config.position;
  line.rotation = Quaternion::FromDirection(direction, up);
  line.scale.x = config.thickness;
  line.scale.y = config.thickness;
  line.scale.z = length;
  line.color = tVec4f(config.color, 0.8f);

  commit(line);
}

void Debug::ShowDebugCone(Tachyon* tachyon, State& state, const DebugConeConfig& config) {
  auto& cone = use_instance(state.meshes.debug_cone);

  tVec3f direction = config.direction;

  tVec3f up = direction.y != 0.f && direction.x == 0.f && direction.z == 0.f
    ? tVec3f(1.f, 0, 0)
    : tVec3f(0, 1.f, 0);

  cone.position = config.position;
  cone.rotation = Quaternion::FromDirection(direction, up);
  cone.scale = config.scale;
  cone.color = tVec4f(config.color, 0.8f);

  commit(cone);
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
  line.color = tVec4f(color, 0.8f);

  commit(line);

  auto& cone = use_instance(state.meshes.debug_cone);

  cone.position = position + vector;
  cone.rotation = line.rotation;
  cone.scale = tVec3f(400.f);
  cone.color = line.color;

  commit(cone);
}

void Debug::ShowDebugPlane(Tachyon* tachyon, State& state, const CollisionPlane& plane, const tVec3f& color) {
  tVec3f midpoint = (plane.p1 + plane.p2 + plane.p3 + plane.p4) / 4.f;
  float x_scale = tVec3f::distance(plane.p1, plane.p4) / 2.f;
  float z_scale = tVec3f::distance(plane.p1, plane.p2) / 2.f;
  tVec3f direction = (plane.p1 - plane.p2).unit();

  auto& debug_plane = use_instance(state.meshes.debug_plane);

  debug_plane.position = midpoint;
  debug_plane.rotation = Quaternion::FromDirection(direction, tVec3f(0, 1.f, 0));
  debug_plane.scale = tVec3f(x_scale, 1.f, z_scale);
  debug_plane.color = color;

  commit(debug_plane);
}