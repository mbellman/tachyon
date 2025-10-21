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

static std::vector<Plane> GetEntityPlanes(const std::vector<GameEntity>& entities) {
  // @allocation
  std::vector<Plane> planes;

  for_entities(entities) {
    auto& entity = entities[i];
    auto plane = CollisionSystem::CreatePlane(entity.position, entity.scale, entity.orientation);

    planes.push_back(plane);
  }

  // @allocation
  return planes;
}

static std::vector<Plane> GetObjectPlanes(Tachyon* tachyon, uint16 mesh_index) {
  // @allocation
  std::vector<Plane> planes;

  for (auto& object : objects(mesh_index)) {
    auto plane = CollisionSystem::CreatePlane(object.position, object.scale, object.rotation);

    planes.push_back(plane);
  }

  // @allocation
  return planes;
}

static bool IsPointOnAnyPlane(const tVec3f& position, const std::vector<Plane>& planes) {
  for (auto& plane : planes) {
    if (CollisionSystem::IsPointOnPlane(position, plane)) {
      return true;
    }
  }

  return false;
}

/* ---------------------------- */

/**
 * ----------------------------
 * Grass
 * ----------------------------
 */
static void GenerateProceduralGrass(Tachyon* tachyon, State& state) {
  log_time("GenerateProceduralGrass()");

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

/**
 * ----------------------------
 * Small grass
 * ----------------------------
 */
static void GenerateProceduralSmallGrass(Tachyon* tachyon, State& state) {
  log_time("GenerateProceduralSmallGrass()");

  remove_all(state.meshes.small_grass);

  auto dirt_path_planes = GetEntityPlanes(state.dirt_paths);

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

      if (IsPointOnAnyPlane(position, dirt_path_planes)) {
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
    std::string message = "Generated " + std::to_string(objects(state.meshes.small_grass).total_active) + " small grass objects";

    console_log(message);
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

    // Restore position after commit to avoid drift
    grass.position.x = base_position.x;
    grass.position.z = base_position.z;
  }
}

/**
 * ----------------------------
 * Ground flowers
 * ----------------------------
 */
static void GenerateProceduralGroundFlowers(Tachyon* tachyon, State& state) {
  log_time("GenerateProceduralGroundFlowers()");

  remove_all(state.meshes.flower);

  auto dirt_path_planes = GetEntityPlanes(state.dirt_paths);
  auto flat_ground_planes = GetObjectPlanes(tachyon, state.meshes.flat_ground);

  for (int i = 0; i < 2000; i++) {
    tVec3f center;
    center.x = Tachyon_GetRandom(-150000.f, 150000.f);
    center.y = -1460.f;
    center.z = Tachyon_GetRandom(-150000.f, 150000.f);

    for (int i = 0; i < 4; i++) {
      tVec3f position;
      position.x = center.x + Tachyon_GetRandom(-1500.f, 1500.f);
      position.y = center.y;
      position.z = center.z + Tachyon_GetRandom(-1500.f, 1500.f);

      if (
        IsPointOnAnyPlane(position, dirt_path_planes) ||
        !IsPointOnAnyPlane(position, flat_ground_planes)
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

  // @todo dev mode only
  {
    std::string message = "Generated " + std::to_string(objects(state.meshes.flower).total_active) + " ground flower objects";

    console_log(message);
  }
}

static void UpdateProceduralGroundFlowers(Tachyon* tachyon, State& state) {
  static const tVec3f offsets[] = {
    tVec3f(0, 0, 0),
    tVec3f(-500.f, 0, 450.f),
    tVec3f(350.f, 0, 400.f),
    tVec3f(600.f, 0, -350.f)
  };

  const float growth_rate = 0.7f;
  const float lifetime = t_PI + t_HALF_PI;

  auto& player_position = state.player_position;
  float base_time_progress = 0.5f * (state.astro_time - -500.f);

  tVec3f sprouting_color = tVec3f(0.1f, 0.6f, 0.1f);
  tVec3f blossom_color = tVec3f(1.f, 0.1f, 0.1f);
  tVec3f wilted_color = tVec3f(0.1f);

  for (auto& flower : objects(state.meshes.flower)) {
    if (abs(flower.position.x - player_position.x) > 15000.f || abs(flower.position.z - player_position.z) > 15000.f) {
      continue;
    }

    float alpha_variation = fmodf(abs(flower.position.x + flower.position.z), 10.f);
    float alpha = base_time_progress + alpha_variation;
    float life_cycles = alpha / lifetime;
    float life_progress = (life_cycles - (int)life_cycles);
    int life_cycle = (int)life_cycles + (int)abs(flower.position.x);
    float growth_factor;
    float wilting_factor;
    tVec3f color;

    if (life_progress < 0.5f) {
      // life_progress 0.0 -> 0.5 => growth_factor 0.0 -> 250.0
      growth_factor = 500.f * life_progress;
      wilting_factor = 0.f;
      color = tVec3f::lerp(sprouting_color, blossom_color, 2.f * life_progress);
    }
    else if (life_progress < 0.8f) {
      growth_factor = 250.f;
      wilting_factor = 0.f;
      color = blossom_color;
    }
    else {
      // life_progress 0.8 -> 1.0 => wilting_factor 0.0 -> 1.0
      wilting_factor = 1.f - 5.f * (1.f - life_progress);
      // life_progress 0.8 -> 1.0 => growth_factor 250.0 -> 200.0
      growth_factor = 250.f - 50.f * wilting_factor;
      color = tVec3f::lerp(blossom_color, wilted_color, wilting_factor);
    }

    tVec3f base_position = flower.position;

    flower.scale.x = growth_factor;
    flower.scale.y = 250.f * (1.f - wilting_factor);
    flower.scale.z = growth_factor;
    flower.color = color;
    flower.position = base_position + offsets[life_cycle % 5];

    commit(flower);

    // Restore position after commit to avoid drift
    flower.position = base_position;
  }
}

/**
 * ----------------------------
 * Bush flowers
 * ----------------------------
 */
static void UpdateBushFlowers(Tachyon* tachyon, State& state) {
  profile("UpdateBushFlowers()");

  auto& player_position = state.player_position;

  for_entities(state.flowers) {
    auto& entity = state.flowers[i];
    auto& position = entity.visible_position;

    if (entity.visible_scale.x < 500.f) {
      continue;
    }

    float distance = tVec3f::distance(entity.visible_position, player_position);

    if (distance < 15000.f) {
      // @todo
    }
  }
}

/* ---------------------------- */

void ProceduralGeneration::RebuildProceduralObjects(Tachyon* tachyon, State& state) {
  // @todo refactor these two
  GenerateProceduralGrass(tachyon, state);
  GenerateProceduralSmallGrass(tachyon, state);
  GenerateProceduralGroundFlowers(tachyon, state);
}

void ProceduralGeneration::UpdateProceduralObjects(Tachyon* tachyon, State& state) {
  profile("UpdateProceduralObjects()");

  // @todo refactor these two
  UpdateProceduralGrass(tachyon, state);
  UpdateProceduralSmallGrass(tachyon, state);

  UpdateProceduralGroundFlowers(tachyon, state);
  UpdateBushFlowers(tachyon, state);
}