#include "astro/procedural_generation.h"

using namespace astro;

static void GenerateProceduralGrass(Tachyon* tachyon, State& state) {
  remove_all(state.meshes.grass);

  for (auto& plane : objects(state.meshes.ground_1)) {
    float x_min = plane.position.x - plane.scale.x * 0.8f;
    float x_max = plane.position.x + plane.scale.x * 0.8f;

    float z_min = plane.position.z - plane.scale.z * 0.8f;
    float z_max = plane.position.z + plane.scale.z * 0.8f;

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
      grass.position.y = -1500.f;
      grass.position.z = fz;

      grass.scale = tVec3f(1000.f);
      grass.color = tVec4f(0.1f, 0.5f, 0.1f, 0.2f);
      grass.material = tVec4f(0.8f, 0, 0, 1.f);

      commit(grass);
    }
  }

  // @todo dev mode only
  {
    std::string message = "Generated " + std::to_string(objects(state.meshes.grass).total_active) + " grass objects";
  
    console_log(message);
  }
}

static void HandleProceduralGrass(Tachyon* tachyon, State& state) {
  profiler_start("HandleProceduralGrass()");

  static const tVec3f offsets[] = {
    tVec3f(0, 0, 0),
    tVec3f(-200.f, 0, 150.f),
    tVec3f(250.f, 0, 300.f),
    tVec3f(100.f, 0, -250.f)
  };

  static const float scales[] = {
    1200.f,
    1000.f,
    1400.f,
    800.f,
    950.f
  };

  const float growth_rate = 0.7f;

  // @todo @optimize only update grass near the player
  for (auto& grass : objects(state.meshes.grass)) {
    float alpha = state.astro_time + grass.position.x + grass.position.z;
    int iteration = int(growth_rate * alpha / t_TAU - 0.8f);
    float rotation_angle = grass.position.x + float(iteration) * 1.3f;

    tVec3f base_position = grass.position;

    grass.position += offsets[iteration % 4];
    grass.scale = tVec3f(scales[iteration % 5]) * sqrtf(0.5f + 0.5f * sinf(growth_rate * alpha));
    grass.rotation = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), rotation_angle);

    commit(grass);

    // Restore the grass position after commit, so its position
    // does not drift over time
    grass.position.x = base_position.x;
    grass.position.z = base_position.z;
  }

  profiler_end();
}

void ProceduralGeneration::RebuildProceduralObjects(Tachyon* tachyon, State& state) {
  GenerateProceduralGrass(tachyon, state);
}

void ProceduralGeneration::HandleProceduralObjects(Tachyon* tachyon, State& state) {
  HandleProceduralGrass(tachyon, state);
}