#include "astro/procedural_generation.h"
#include "astro/collision_system.h"

using namespace astro;

struct Bounds2D {
  float x[2];
  float z[2];
};

static Bounds2D GetObjectBounds2D(const tObject& object, const float scale_factor) {
  Bounds2D bounds;

  bounds.x[0] = object.position.x - object.scale.x * scale_factor;
  bounds.x[1] = object.position.x + object.scale.x * scale_factor;

  bounds.z[0] = object.position.z - object.scale.z * scale_factor;
  bounds.z[1] = object.position.z + object.scale.z * scale_factor;

  return bounds;
}

static std::vector<Plane>& GetDirtPathPlanes(Tachyon* tachyon, State& state) {
  static std::vector<Plane> planes;

  planes.clear();

  for_entities(state.dirt_paths) {
    auto& path = state.dirt_paths[i];
    auto plane = CollisionSystem::CreatePlane(path.position, path.scale, path.orientation);

    planes.push_back(plane);
  }

  return planes;
}

static bool IsWithinAnyPlane(const std::vector<Plane>& planes, const tVec3f& position) {
  for (auto& plane : planes) {
    if (CollisionSystem::IsPointOnPlane(position, plane)) {
      return true;
    }
  }

  return false;
}

static void GenerateProceduralGrass(Tachyon* tachyon, State& state) {
  remove_all(state.meshes.grass);

  // @todo factor
  for (auto& ground : objects(state.meshes.ground_1)) {
    auto bounds = GetObjectBounds2D(ground, 0.8f);
    tVec3f direction = ground.rotation.getDirection();
    float theta = atan2f(direction.z, direction.x) + t_HALF_PI;

    for (uint16 i = 0; i < 20; i++) {
      auto& grass = create(state.meshes.grass);

      float x = Tachyon_GetRandom(bounds.x[0], bounds.x[1]);
      float z = Tachyon_GetRandom(bounds.z[0], bounds.z[1]);

      float lx = x - ground.position.x;
      float lz = z - ground.position.z;

      float wx = ground.position.x + (lx * cosf(theta) - lz * sinf(theta));
      float wz = ground.position.z + (lx * sinf(theta) + lz * cosf(theta));

      grass.position.x = wx;
      grass.position.y = ground.position.y + 0.2f * ground.scale.y;
      grass.position.z = wz;

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
  auto start = Tachyon_GetMicroseconds();

  remove_all(state.meshes.small_grass);

  auto& dirt_path_planes = GetDirtPathPlanes(tachyon, state);

  // @todo factor
  for (auto& plane : objects(state.meshes.flat_ground)) {
    auto bounds = GetObjectBounds2D(plane, 0.95f);
    float area = (bounds.x[1] - bounds.x[0]) * (bounds.z[1] - bounds.z[0]);
    uint16 total_grass = uint16(area / 1500000.f);

    tVec3f direction = plane.rotation.getDirection();
    float theta = atan2f(direction.z, direction.x) + t_HALF_PI;

    for (uint16 i = 0; i < total_grass; i++) {
      float x = Tachyon_GetRandom(bounds.x[0], bounds.x[1]);
      float z = Tachyon_GetRandom(bounds.z[0], bounds.z[1]);

      float lx = x - plane.position.x;
      float lz = z - plane.position.z;

      float wx = plane.position.x + (lx * cosf(theta) - lz * sinf(theta));
      float wz = plane.position.z + (lx * sinf(theta) + lz * cosf(theta));

      tVec3f position = tVec3f(wx, -1500.f, wz);

      if (IsWithinAnyPlane(dirt_path_planes, position)) {
        continue;
      }

      auto& grass = create(state.meshes.small_grass);

      grass.position = position;
      grass.scale = tVec3f(Tachyon_GetRandom(200.f, 500.f));
      grass.color = tVec4f(0.2f, 0.8f, 0.2f, 0.2f);
      grass.rotation = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), Tachyon_GetRandom(0.f, t_TAU));
      grass.material = tVec4f(0.8f, 0, 0, 1.f);

      commit(grass);
    }
  }

  // @todo dev mode only
  {
    uint64 duration = Tachyon_GetMicroseconds() - start;

    std::string message = "Generated " + std::to_string(objects(state.meshes.small_grass).total_active) + " small grass objects (" + std::to_string(duration) + "us)";

    console_log(message);
  }
}

// @todo UpdateProceduralFlowers()
static void GenerateProceduralFlowers(Tachyon* tachyon, State& state) {
  remove_all(state.meshes.flower);

  auto& dirt_path_planes = GetDirtPathPlanes(tachyon, state);

  // @temporary
  std::vector<Plane> flat_ground_planes;

  for (auto& ground : objects(state.meshes.flat_ground)) {
    auto plane = CollisionSystem::CreatePlane(ground.position, ground.scale, ground.rotation);

    flat_ground_planes.push_back(plane);
  }

  for (int i = 0; i < 1000; i++) {
    // @todo use clustering behavior
    tVec3f position;
    position.x = Tachyon_GetRandom(-50000.f, 50000.f);
    position.y = -1470.f;
    position.z = Tachyon_GetRandom(-50000.f, 50000.f);

    if (
      IsWithinAnyPlane(dirt_path_planes, position) ||
      !IsWithinAnyPlane(flat_ground_planes, position)
    ) {
      continue;
    }

    auto& flower = create(state.meshes.flower);

    flower.position = position;
    flower.scale = tVec3f(250.f);
    flower.color = tVec3f(1.f, 0.1f, 0.1f);

    commit(flower);
  }
}

static void UpdateProceduralGrass(Tachyon* tachyon, State& state) {
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
    int iteration = (int)abs(growth_rate * alpha / t_TAU - 0.8f);
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
  static const tVec3f offsets[] = {
    tVec3f(0, 0, 0),
    tVec3f(-200.f, 0, 150.f),
    tVec3f(250.f, 0, 300.f),
    tVec3f(100.f, 0, -250.f)
  };

  static const float scales[] = {
    300.f,
    600.f,
    500.f,
    450.f,
    400.f
  };

  const float growth_rate = 0.7f;

  auto& player_position = state.player_position;

  for (auto& grass : objects(state.meshes.small_grass)) {
    if (abs(grass.position.x - player_position.x) > 15000.f || abs(grass.position.z - player_position.z) > 15000.f) {
      continue;
    }

    float alpha = state.astro_time + grass.position.x + grass.position.z;
    int iteration = (int)abs(growth_rate * alpha / t_TAU - 0.8f);
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
  GenerateProceduralFlowers(tachyon, state);
}

void ProceduralGeneration::UpdateProceduralObjects(Tachyon* tachyon, State& state) {
  profile("UpdateProceduralObjects()");

  // @todo refactor these two
  UpdateProceduralGrass(tachyon, state);
  UpdateProceduralSmallGrass(tachyon, state);
}