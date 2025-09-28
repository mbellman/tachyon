#include "astro/procedural_generation.h"

using namespace astro;

static void GenerateProceduralGrass(Tachyon* tachyon, State& state) {
  remove_all(state.meshes.grass);

  for (auto& plane : objects(state.meshes.ground_1)) {
    float x_min = plane.position.x - plane.scale.x;
    float x_max = plane.position.x + plane.scale.x;

    float z_min = plane.position.z - plane.scale.z;
    float z_max = plane.position.z + plane.scale.z;

    tVec3f direction = plane.rotation.getDirection();
    float theta = atan2f(direction.z, direction.x) + t_HALF_PI;

    for (uint16 i = 0; i < 20; i++) {
      auto& grass = create(state.meshes.grass);

      float x = Tachyon_GetRandom(x_min, x_max);
      float z = Tachyon_GetRandom(z_min, z_max);

      float mx = x - plane.position.x;
      float mz = z - plane.position.z;

      float fx = mx;
      float fz = mz;

      fx = plane.position.x + (mx * cosf(theta) - mz * sinf(theta));
      fz = plane.position.z + (mx * sinf(theta) + mz * cosf(theta));

      grass.position.x = fx;
      grass.position.y = -1200.f;
      grass.position.z = fz;

      grass.scale = tVec3f(1000.f);
      grass.color = tVec3f(0.1f, 0.7f, 0.1f);

      commit(grass);
    }
  }

  printf("Generated %d grass objects\n", objects(state.meshes.grass).total_active);
}

static void HandleProceduralGrass(Tachyon* tachyon, State& state) {
  float growth_rate = 0.6f;

  // @todo only update grass near the player
  for (auto& grass : objects(state.meshes.grass)) {
    float alpha = state.astro_time + grass.position.x;
    float angle = grass.position.x + float(int(growth_rate * alpha / t_TAU - 0.8f)) * 1.3f;

    grass.position.y = -1500.f;
    grass.scale = tVec3f(1200.f) * (0.5f + 0.5f * sinf(growth_rate * alpha));
    grass.rotation = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), angle);

    commit(grass);
  }
}

void ProceduralGeneration::RebuildProceduralObjects(Tachyon* tachyon, State& state) {
  GenerateProceduralGrass(tachyon, state);
}

void ProceduralGeneration::HandleProceduralObjects(Tachyon* tachyon, State& state) {
  HandleProceduralGrass(tachyon, state);
}