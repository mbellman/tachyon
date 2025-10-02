#include "astro/procedural_generation.h"

using namespace astro;

static void GenerateProceduralGrass(Tachyon* tachyon, State& state) {
  remove_all(state.meshes.grass);

  for (auto& ground : objects(state.meshes.ground_1)) {
    float x_min = ground.position.x - ground.scale.x * 0.8f;
    float x_max = ground.position.x + ground.scale.x * 0.8f;

    float z_min = ground.position.z - ground.scale.z * 0.8f;
    float z_max = ground.position.z + ground.scale.z * 0.8f;

    tVec3f direction = ground.rotation.getDirection();
    float theta = atan2f(direction.z, direction.x) + t_HALF_PI;

    for (uint16 i = 0; i < 20; i++) {
      auto& grass = create(state.meshes.grass);

      float x = Tachyon_GetRandom(x_min, x_max);
      float z = Tachyon_GetRandom(z_min, z_max);

      float mx = x - ground.position.x;
      float mz = z - ground.position.z;

      float fx = mx;
      float fz = mz;

      fx = ground.position.x + (mx * cosf(theta) - mz * sinf(theta));
      fz = ground.position.z + (mx * sinf(theta) + mz * cosf(theta));

      grass.position.x = fx;
      grass.position.y = -1500.f;
      grass.position.z = fz;

      grass.scale = tVec3f(1000.f);
      grass.color = tVec4f(0.1f, 0.3f, 0.1f, 0.2f);
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

static void GenerateProceduralSmallGrass(Tachyon* tachyon, State& state) {
  remove_all(state.meshes.small_grass);

  for (auto& plane : objects(state.meshes.flat_ground)) {
    float x_min = plane.position.x - plane.scale.x * 0.8f;
    float x_max = plane.position.x + plane.scale.x * 0.8f;

    float z_min = plane.position.z - plane.scale.z * 0.8f;
    float z_max = plane.position.z + plane.scale.z * 0.8f;

    float area = (x_max - x_min) * (z_max - z_min);

    uint16 total_grass = uint16(area / 2000000.f);

    for (uint16 i = 0; i < total_grass; i++) {
      auto& grass = create(state.meshes.small_grass);

      grass.position.x = Tachyon_GetRandom(x_min, x_max);
      grass.position.y = -1500.f;
      grass.position.z = Tachyon_GetRandom(z_min, z_max);

      grass.scale = tVec3f(Tachyon_GetRandom(200.f, 500.f));
      grass.color = tVec4f(0.2f, 0.8f, 0.2f, 0.2f);
      grass.rotation = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), Tachyon_GetRandom(0.f, t_TAU));
      grass.material = tVec4f(0.8f, 0, 0, 1.f);
 
      commit(grass);
    }
  }

  // @todo dev mode only
  {
    std::string message = "Generated " + std::to_string(objects(state.meshes.small_grass).total_active) + " small grass objects";
  
    console_log(message);
  }
}

static void UpdateProceduralGrass(Tachyon* tachyon, State& state) {
  profile("UpdateProceduralGrass()");

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

  auto& player_position = state.player_position;

  for (auto& grass : objects(state.meshes.grass)) {
    if (abs(grass.position.x - player_position.x) > 15000.f || abs(grass.position.z - player_position.z) > 15000.f) {
      continue;
    }

    float alpha = state.astro_time + grass.position.x + grass.position.z;
    int iteration = int(growth_rate * alpha / t_TAU - 0.8f);
    float rotation_angle = grass.position.x + float(iteration) * 1.3f;

    tVec3f base_position = grass.position;

    grass.position += offsets[iteration % 4];
    grass.scale = tVec3f(scales[iteration % 5]) * 1.2f * sqrtf(0.5f + 0.5f * sinf(growth_rate * alpha));
    grass.rotation = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), rotation_angle);

    commit(grass);

    // Restore the grass position after commit, so its position
    // does not drift over time
    grass.position.x = base_position.x;
    grass.position.z = base_position.z;
  }
}

static void UpdateProceduralSmallGrass(Tachyon* tachyon, State& state) {
  profile("UpdateProceduralSmallGrass()");

  static const tVec3f offsets[] = {
    tVec3f(0, 0, 0),
    tVec3f(-200.f, 0, 150.f),
    tVec3f(250.f, 0, 300.f),
    tVec3f(100.f, 0, -250.f)
  };

  static const float scales[] = {
    200.f,
    500.f,
    300.f,
    400.f,
    250.f
  };

  const float growth_rate = 0.7f;

  auto& player_position = state.player_position;

  for (auto& grass : objects(state.meshes.small_grass)) {
    if (abs(grass.position.x - player_position.x) > 15000.f || abs(grass.position.z - player_position.z) > 15000.f) {
      continue;
    }

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
}

void ProceduralGeneration::RebuildProceduralObjects(Tachyon* tachyon, State& state) {
  // @todo refactor these two
  GenerateProceduralGrass(tachyon, state);
  GenerateProceduralSmallGrass(tachyon, state);
}

void ProceduralGeneration::UpdateProceduralObjects(Tachyon* tachyon, State& state) {
  // @todo refactor these two
  UpdateProceduralGrass(tachyon, state);
  UpdateProceduralSmallGrass(tachyon, state);
}