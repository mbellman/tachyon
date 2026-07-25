#include "engine/tachyon.h"

#include "metro/debug.h"

using namespace metro;

void Debug::HandleFrameStart(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  reset_instances(meshes.debug_ring);
  reset_instances(meshes.debug_sphere);
}

void Debug::ShowDebugSphere(Tachyon* tachyon, State& state, const tVec3f& position, const float radius) {
  auto& sphere = use_instance(state.meshes.debug_sphere);

  sphere.position = position;
  sphere.scale = tVec3f(radius);
  sphere.color = 0x00F8;

  commit(sphere);
}