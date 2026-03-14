#include "astro/procedural_growth.h"

using namespace astro;

void ProceduralBehavior::Growth::UpdateWhiteVines(Tachyon* tachyon, State& state) {
  profile("UpdateWhiteVines()");
  auto& meshes = state.meshes;

  reset_instances(meshes.vine_leaf);

  uint16 total_oak_trunks = mesh(meshes.oak_tree_trunk).lod_1.instance_count;

  for (uint16 i = 0; i < total_oak_trunks; i++) {
    auto& trunk = objects(meshes.oak_tree_trunk)[i];

    if (abs(state.player_position.x - trunk.position.x) > 20000.f) continue;
    if (abs(state.player_position.z - trunk.position.z) > 15000.f) continue;

    for (int i = 0; i < 30; i++) {
      auto& leaf = use_instance(meshes.vine_leaf);

      float angle = sinf(float(i) * 0.5f);
      float tilt = 0.5f * cosf(float(i));

      Quaternion rotation = (
        Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), t_PI + angle) *
        Quaternion::fromAxisAngle(tVec3f(0, 0, 1.f), tilt)
      );

      tVec3f offset;
      offset.x = trunk.scale.x * 0.38f * sinf(angle);
      offset.y = -trunk.scale.y * 0.5f;
      offset.z = trunk.scale.z * 0.38f * cosf(angle);

      leaf.position = trunk.position + offset;
      leaf.position.y += float(i) * 250.f;
      leaf.scale = tVec3f(250.f);
      leaf.rotation = rotation;
      leaf.color = tVec3f(0.1f, 0.3f, 0.2f);
      leaf.material = tVec4f(0.4f, 0, 0, 1.f);

      commit(leaf);
    }
  }
}