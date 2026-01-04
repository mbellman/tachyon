#include <algorithm>
#include <execution>
#include <functional>
#include <ranges>

#include "astro/procedural_generation.h"
#include "astro/astrolabe.h"
#include "astro/collision_system.h"
#include "astro/entity_behaviors/behavior.h"
#include "astro/path_generation.h"
#include "astro/procedural_trees.h"

using namespace astro;

#define parallel_for_range(__start, __end, ...)\
  {\
    std::ranges::iota_view __range((int)__start, (int)__end);\
    std::for_each(std::execution::par, __range.begin(), __range.end(), [&, tachyon](size_t __index){\
      __VA_ARGS__\
    });\
  }\

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

static std::vector<Plane> GetEntityPlanes(const std::vector<GameEntity>& entities, const tVec3f& scale_factor = tVec3f(1.f)) {
  // @allocation
  std::vector<Plane> planes;

  for_entities(entities) {
    auto& entity = entities[i];
    auto plane = CollisionSystem::CreatePlane(entity.position, entity.scale * scale_factor, entity.orientation);

    planes.push_back(plane);
  }

  // @allocation
  return planes;
}

static std::vector<Plane> GetObjectPlanes(Tachyon* tachyon, uint16 mesh_index, const tVec3f& scale_factor = tVec3f(1.f)) {
  // @allocation
  std::vector<Plane> planes;

  for (auto& object : objects(mesh_index)) {
    auto plane = CollisionSystem::CreatePlane(object.position, object.scale * scale_factor, object.rotation);

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

struct FlowerBloomParameters {
  // @todo
};

static void UpdateBloomingFlower(tObject& flower, const tVec3f& blossom_color, const float max_size, const float alpha, const float lifetime) {
  const tVec3f sprouting_color = tVec3f(0.1f, 0.6f, 0.1f);
  const tVec3f wilted_color = tVec3f(0.2f, 0.1f, 0.1f);

  float life_cycles = alpha / lifetime;
  float life_progress = (life_cycles - (int)life_cycles);
  float growth_factor;
  float wilting_factor;
  tVec3f color;

  if (life_progress < 0.5f) {
    // life_progress 0.0 -> 0.5 => growth_factor 0.0 -> 1.0
    growth_factor = 2.f * life_progress;
    wilting_factor = 0.f;
    color = tVec3f::lerp(sprouting_color, blossom_color, 2.f * life_progress);
  }
  else if (life_progress < 0.8f) {
    growth_factor = 1.f;
    wilting_factor = 0.f;
    color = blossom_color;
  }
  else {
    // life_progress 0.8 -> 1.0 => wilting_factor 0.0 -> 1.0
    wilting_factor = 1.f - 5.f * (1.f - life_progress);
    // life_progress 0.8 -> 1.0 => growth_factor 1.0 -> 0.6
    growth_factor = 1.f - 0.4f * wilting_factor;
    color = tVec3f::lerp(blossom_color, wilted_color, wilting_factor);
  }

  flower.scale.x = growth_factor * max_size;
  flower.scale.y = max_size * (1.f - 0.9f * wilting_factor);
  flower.scale.z = growth_factor * max_size;
  flower.color = color;
}

/* ---------------------------- */

/**
 * ----------------------------
 * ground_1 plants
 * ----------------------------
 */
static void GenerateGround1Plants(Tachyon* tachyon, State& state) {
  log_time("GenerateGround1Plants()");

  // Reset objects
  {
    remove_all(state.meshes.grass);
    remove_all(state.meshes.ground_1_flower);

    for (uint16 i = 0; i < 10000; i++) {
      create(state.meshes.grass);
    }

    for (uint16 i = 0; i < 1000; i++) {
      create(state.meshes.ground_1_flower);
    }
  }

  // Reset clusters
  {
    for (auto& cluster : state.ground_plant_clusters) {
      cluster.grass_positions.clear();
      cluster.flower_positions.clear();
    }

    state.ground_plant_clusters.clear();
  }

  // Create new clusters
  for (auto& ground : objects(state.meshes.ground_1)) {
    if (ground.position.y < -2500.f) continue;

    auto bounds = GetObjectBounds2D(ground, 0.8f);
    tVec3f direction = ground.rotation.getDirection();
    float theta = atan2f(direction.z, direction.x) + t_HALF_PI;

    GroundPlantCluster cluster;
    cluster.position = ground.position;

    // Generate grass pieces
    for (uint16 i = 0; i < 50; i++) {
      // @todo factor
      float x = Tachyon_GetRandom(bounds.x[0], bounds.x[1]);
      float z = Tachyon_GetRandom(bounds.z[0], bounds.z[1]);

      float lx = x - ground.position.x;
      float lz = z - ground.position.z;

      float wx = ground.position.x + (lx * cosf(theta) - lz * sinf(theta));
      float wz = ground.position.z + (lx * sinf(theta) + lz * cosf(theta));

      float adjusted_scale = (ground.scale.x + ground.scale.z) / 2.f;
      float height_alpha = 1.f - sqrtf(lx*lx + lz*lz) / adjusted_scale;
      float y_factor = Tachyon_Lerpf(0.2f, 0.7f, height_alpha);

      cluster.grass_positions.push_back({
        wx,
        ground.position.y + y_factor * ground.scale.y,
        wz
      });
    }

    // Generate flowers
    for (uint16 i = 0; i < 5; i++) {
      // @todo factor
      float x = Tachyon_GetRandom(bounds.x[0], bounds.x[1]);
      float z = Tachyon_GetRandom(bounds.z[0], bounds.z[1]);

      float lx = x - ground.position.x;
      float lz = z - ground.position.z;

      float wx = ground.position.x + (lx * cosf(theta) - lz * sinf(theta));
      float wz = ground.position.z + (lx * sinf(theta) + lz * cosf(theta));

      float adjusted_scale = (ground.scale.x + ground.scale.z) / 2.f;
      float height_alpha = 1.f - sqrtf(lx*lx + lz*lz) / adjusted_scale;
      float y_factor = Tachyon_Lerpf(0.4f, 0.7f, height_alpha);

      cluster.flower_positions.push_back({
        wx,
        ground.position.y + y_factor * ground.scale.y,
        wz
      });
    }

    // Save the cluster
    state.ground_plant_clusters.push_back(cluster);
  }

  // @todo dev mode only
  {
    std::string message = "Generated " + std::to_string(state.ground_plant_clusters.size()) + " grass clusters";

    console_log(message);
  }
}

static void UpdateGround1Plants(Tachyon* tachyon, State& state) {
  profile("UpdateGround1Plants()");

  static const tVec3f offsets[] = {
    tVec3f(0, 0, 0),
    tVec3f(-200.f, 0, 150.f),
    tVec3f(250.f, 0, 300.f),
    tVec3f(100.f, 0, -250.f)
  };

  static const float scales[] = {
    700.f,
    800.f,
    750.f,
    650.f,
    600.f
  };

  const float growth_rate = 0.7f;

  auto& player_position = state.player_position;

  uint16 total_grass = 0;
  uint16 total_flowers = 0;

  for (auto& cluster : state.ground_plant_clusters) {
    // @todo factor
    if (abs(cluster.position.x - player_position.x) > 20000.f) continue;
    if (abs(cluster.position.z - player_position.z) > 20000.f) continue;

    for (auto& position : cluster.grass_positions) {
      // @todo factor
      float alpha = state.astro_time + position.x + position.z;
      int iteration = (int)abs(growth_rate * alpha / t_TAU - 0.8f);
      float rotation_angle = position.x + float(iteration) * 1.3f;

      float scale_factor = 0.5f + (position.y - -1500.f) * 0.001f;
      if (scale_factor > 1.5f) scale_factor = 1.5f;

      auto& grass = objects(state.meshes.grass)[total_grass++];

      grass.position = position + offsets[iteration % 4];
      grass.scale = tVec3f(scales[iteration % 5]) * sqrtf(0.5f + 0.5f * sinf(growth_rate * alpha));
      grass.scale *= scale_factor;

      grass.color = tVec4f(0.2f, 0.3f, 0.1f, 0.3f);
      grass.material = tVec4f(0.8f, 0, 0, 1.f);

      grass.rotation = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), rotation_angle);

      commit(grass);
    }

    float emissivity = state.is_nighttime ? 0.6f : 0.f;

    for (auto& position : cluster.flower_positions) {
      // @todo factor
      float alpha = state.astro_time + position.x + position.z;
      int iteration = (int)abs(growth_rate * alpha / t_TAU - 0.8f);
      float rotation_angle = position.x + float(iteration) * 1.3f;

      float scale_factor = 0.5f + (position.y - -1500.f) * 0.001f;
      if (scale_factor > 1.5f) scale_factor = 1.5f;
      scale_factor *= 0.2f;

      auto& flower = objects(state.meshes.ground_1_flower)[total_flowers++];

      flower.position = position + offsets[iteration % 4];
      flower.scale = tVec3f(scales[iteration % 5]) * sqrtf(0.5f + 0.5f * sinf(growth_rate * alpha));
      flower.scale *= scale_factor;

      flower.color = tVec4f(0.8f, 0.3f, 0.3f, emissivity);
      flower.material = tVec4f(0.8f, 0, 0, 0.7f);

      flower.rotation = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), rotation_angle);

      commit(flower);
    }
  }

  mesh(state.meshes.grass).lod_1.instance_count = total_grass;
  mesh(state.meshes.ground_1_flower).lod_1.instance_count = total_flowers;

  // @todo dev mode only
  {
    add_dev_label("  Total grass: ", std::to_string(total_grass));
  }
}

/**
 * ----------------------------
 * Small grass
 * ----------------------------
 */
static void GenerateSmallGrass(Tachyon* tachyon, State& state) {
  log_time("GenerateSmallGrass()");

  auto& meshes = state.meshes;

  const float chunk_width = 20000.f;
  const float chunk_height = 20000.f;

  // @allocation
  auto flat_ground_planes = GetObjectPlanes(tachyon, meshes.flat_ground);
  auto ground_1_planes = GetObjectPlanes(tachyon, meshes.ground_1, tVec3f(0.9f));
  auto altar_planes = GetEntityPlanes(state.altars, tVec3f(1.9f, 1.f, 0.6f));
  auto wind_chime_planes = GetEntityPlanes(state.wind_chimes, tVec3f(0.8f, 1.f, 1.4f));

  // Reset objects/chunks/etc.
  {
    remove_all(meshes.small_grass);

    for (auto& chunk : state.grass_chunks) {
      chunk.grass_blades.clear();
    }

    state.grass_chunks.clear();
  }

  // Generate small grass chunks
  for (int32 x = -20; x <= 20; x++) {
    for (int32 z = -20; z <= 20; z++) {
      tVec3f center_position = tVec3f(x * chunk_width, 0.f, z * chunk_height);
      tVec3f upper_left_corner = center_position + tVec3f(-chunk_width * 0.5f, 0, -chunk_height * 0.5f);
      tVec3f upper_right_corner = center_position + tVec3f(chunk_width * 0.5f, 0, -chunk_height * 0.5f);
      tVec3f lower_left_corner = center_position + tVec3f(-chunk_width * 0.5f, 0, chunk_height * 0.5f);
      tVec3f lower_right_corner = center_position + tVec3f(chunk_width * 0.5f, 0, chunk_height * 0.5f);

      if (
        !IsPointOnAnyPlane(center_position, flat_ground_planes) &&
        !IsPointOnAnyPlane(upper_left_corner, flat_ground_planes) &&
        !IsPointOnAnyPlane(upper_right_corner, flat_ground_planes) &&
        !IsPointOnAnyPlane(lower_left_corner, flat_ground_planes) &&
        !IsPointOnAnyPlane(lower_right_corner, flat_ground_planes)
      ) {
        continue;
      }

      GrassChunk chunk;
      chunk.center_position = center_position;

      state.grass_chunks.push_back(chunk);
    }
  }

  // Add blades to each chunk in parallel
  parallel_for_range(0, state.grass_chunks.size(), {
    auto& chunk = state.grass_chunks[__index];

    tVec3f upper_left_corner = chunk.center_position + tVec3f(-chunk_width * 0.5f, 0, -chunk_height * 0.5f);
    tVec3f upper_right_corner = chunk.center_position + tVec3f(chunk_width * 0.5f, 0, -chunk_height * 0.5f);
    tVec3f lower_left_corner = chunk.center_position + tVec3f(-chunk_width * 0.5f, 0, chunk_height * 0.5f);
    tVec3f lower_right_corner = chunk.center_position + tVec3f(chunk_width * 0.5f, 0, chunk_height * 0.5f);

    // @allocation
    std::vector<Plane> local_ground_1_planes;
    std::vector<PathSegment> local_dirt_path_segments;
    std::vector<PathSegment> local_stone_path_segments;

    for (auto& plane : ground_1_planes) {
      float distance = tVec3f::distance(plane.p1, chunk.center_position);

      if (distance < chunk_width * 1.2f) {
        local_ground_1_planes.push_back(plane);
      }
    }

    // @todo factor
    for (auto& segment : state.dirt_path_segments) {
      float distance = tVec3f::distance(segment.base_position, chunk.center_position);

      if (abs(segment.base_position.x - chunk.center_position.x) > chunk_width) continue;
      if (abs(segment.base_position.z - chunk.center_position.z) > chunk_height) continue;

      local_dirt_path_segments.push_back(segment);
    }

    // @todo factor
    for (auto& segment : state.stone_path_segments) {
      float distance = tVec3f::distance(segment.base_position, chunk.center_position);

      if (abs(segment.base_position.x - chunk.center_position.x) > chunk_width) continue;
      if (abs(segment.base_position.z - chunk.center_position.z) > chunk_height) continue;

      local_stone_path_segments.push_back(segment);
    }

    chunk.grass_blades.reserve(4500);

    for (int32 i = 0; i < 4500; i++) {
      GrassBlade blade;
      blade.position.x = Tachyon_GetRandom(upper_left_corner.x, upper_right_corner.x);
      blade.position.y = -1500.f;
      blade.position.z = Tachyon_GetRandom(upper_left_corner.z, lower_left_corner.z);

      blade.scale = tVec3f(Tachyon_GetRandom(500.f, 1500.f));
      blade.scale.y *= 0.75f;

      if (!IsPointOnAnyPlane(blade.position, flat_ground_planes)) continue;
      if (IsPointOnAnyPlane(blade.position, local_ground_1_planes)) continue;
      if (IsPointOnAnyPlane(blade.position, altar_planes)) continue;
      if (IsPointOnAnyPlane(blade.position, wind_chime_planes)) continue;

      // @todo factor
      for (auto& segment : local_dirt_path_segments) {
        if (CollisionSystem::IsPointOnPlane(blade.position, segment.plane)) {
          blade.dirt_path_segment_index = segment.index;

          break;
        }
      }

      // @todo factor
      for (auto& segment : local_stone_path_segments) {
        if (CollisionSystem::IsPointOnPlane(blade.position, segment.plane)) {
          blade.stone_path_segment_index = segment.index;

          break;
        }
      }

      chunk.grass_blades.push_back(blade);
    }
  });

  // Count total blades
  int32 total_blades = 0;

  for (size_t i = 0; i < state.grass_chunks.size(); i++) {
    total_blades += state.grass_chunks[i].grass_blades.size();
  }

  // @todo dev mode only
  {
    std::string message = "Generated " + std::to_string(state.grass_chunks.size()) + " grass chunks (" + std::to_string(total_blades) + " blades)";

    console_log(message);
  }
}

static void UpdateSmallGrassObjectByTime(tObject& grass, float astro_time) {
  const static float growth_rate = 0.7f;

  const static tColor colors[] = {
    tVec4f(0.2f, 0.5f, 0.1f, 0.1f),
    tVec4f(0.3f, 0.6f, 0.1f, 0.1f),
    tVec4f(0.1f, 0.4f, 0.1f, 0.1f),
    tVec4f(0.1f, 0.5f, 0.1f, 0.1f)
  };

  // const static tColor colors[] = {
  //   tVec4f(0.5f, 0.3f, 0.1f, 0.1f),
  //   tVec4f(0.6f, 0.4f, 0.1f, 0.1f),
  //   tVec4f(0.4f, 0.3f, 0.1f, 0.1f),
  //   tVec4f(0.5f, 0.4f, 0.1f, 0.1f)
  // };

  float alpha = astro_time + grass.position.x + grass.position.z;
  int iteration = (int)abs(growth_rate * alpha / t_TAU - 0.8f);
  float rotation_angle = float(iteration) * 1.3f;
  auto variation_index = iteration % 4;

  // @todo cache this
  grass.rotation = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), rotation_angle);
  grass.color = colors[variation_index];
}

static void UpdateSmallGrass(Tachyon* tachyon, State& state) {
  profile("UpdateSmallGrass()");

  auto& meshes = state.meshes;

  auto& player_position = state.player_position;
  bool is_astro_turning = state.astro_turn_speed != 0.f;
  auto& small_grass = objects(meshes.small_grass);
  uint16 object_index = 0;

  #define remove_blade_if_active(blade)\
    if (blade.active_object_id != 0xFFFF) {\
      Tachyon_RemoveObject(tachyon, meshes.small_grass, blade.active_object_id);\
      blade.active_object_id = 0xFFFF;\
    }\

  // @todo factor
  auto& camera = tachyon->scene.camera;
  Quaternion standard_camera_rotation = Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), 0.9f);
  // @hack Invert y to get the proper direction. Probably a mistake somewhere.
  tVec3f camera_direction = standard_camera_rotation.getDirection() * tVec3f(1.f, -1.f, 1.f);
  float camera_height = camera.position.y - -1500.f;
  float distance = -camera_height / camera_direction.y;
  tVec3f ground_center = camera.position + camera_direction * distance;

  for (auto& chunk : state.grass_chunks) {
    bool is_chunk_in_view = (
      abs(ground_center.x - chunk.center_position.x) < 28000.f &&
      abs(ground_center.z - chunk.center_position.z) < 25000.f
    );

    if (!is_chunk_in_view) {
      if (chunk.is_currently_in_view) {
        for (auto& blade : chunk.grass_blades) {
          remove_blade_if_active(blade);
        }
      }

      chunk.is_currently_in_view = false;

      continue;
    }

    chunk.is_currently_in_view = true;

    for (auto& blade : chunk.grass_blades) {
      float z_distance = blade.position.z - ground_center.z;
      float x_limit = 15000.f + -z_distance * 0.5f;
      if (x_limit > 20000.f) x_limit = 20000.f;

      // Remove out-of-view blades
      if (
        abs(blade.position.x - ground_center.x) > x_limit ||
        z_distance > 8000.f ||
        z_distance < -18000.f
      ) {
        remove_blade_if_active(blade);

        continue;
      }

      // Remove blades covered up by path segments in the current time
      {
        // @todo factor
        if (blade.dirt_path_segment_index > -1) {
          auto& segment = state.dirt_path_segments[blade.dirt_path_segment_index];

          if (CollisionSystem::IsPointOnPlane(blade.position, segment.plane)) {
            remove_blade_if_active(blade);

            continue;
          }
        }

        // @todo factor
        if (blade.stone_path_segment_index > -1) {
          auto& segment = state.stone_path_segments[blade.stone_path_segment_index];

          if (CollisionSystem::IsPointOnPlane(blade.position, segment.plane)) {
            remove_blade_if_active(blade);

            continue;
          }
        }
      }

      if (blade.active_object_id != 0xFFFF) {
        if (is_astro_turning) {
          // Update any active blades while astro turning,
          // since the blades need to move about in place
          auto& grass = small_grass.getByIdFast(blade.active_object_id);

          // Time evolution
          UpdateSmallGrassObjectByTime(grass, state.astro_time);

          commit(grass);
        }

        continue;
      }

      // Create new small grass objects as the blades stream into view
      auto& grass = create(meshes.small_grass);

      // Time-invariant grass properties
      grass.position = blade.position;
      grass.position.y = -1500.f;
      grass.scale = blade.scale;
      grass.material = tVec4f(0.6f, 0, 0, 0.1f);

      // Time evolution
      UpdateSmallGrassObjectByTime(grass, state.astro_time);

      commit(grass);

      blade.active_object_id = grass.object_id;
    }
  }

  // @todo dev mode only
  {
    uint16 total_active = objects(meshes.small_grass).total_active;

    add_dev_label("  Total small grass: ", std::to_string(total_active));
  }
}

/**
 * ----------------------------
 * Ground flowers
 * ----------------------------
 */
static void GenerateGroundFlowers(Tachyon* tachyon, State& state) {
  log_time("GenerateGroundFlowers()");

  auto& meshes = state.meshes;

  auto dirt_path_planes = GetObjectPlanes(tachyon, meshes.dirt_path);
  auto stone_path_planes = GetObjectPlanes(tachyon, meshes.stone_path);
  auto flat_ground_planes = GetObjectPlanes(tachyon, meshes.flat_ground);

  // @todo factor
  remove_all(meshes.ground_flower);

  // Clusters
  parallel_for_range(0, 7000, {
    tRNG rng(1234.f + float(__index));

    tVec3f center;
    center.x = rng.Random(-300000.f, 300000.f);
    center.y = -1000.f;
    center.z = rng.Random(-300000.f, 300000.f);

    // 4 flowers per cluster
    // @todo avoid crashing at the limit! right now we only avoid
    // hitting the max because there isn't enough flat ground to
    // spawn flowers on
    for (int i = 0; i < 4; i++) {
      tVec3f position;
      position.x = center.x + rng.Random(-1500.f, 1500.f);
      position.y = center.y;
      position.z = center.z + rng.Random(-1500.f, 1500.f);

      if (
        IsPointOnAnyPlane(position, dirt_path_planes) ||
        IsPointOnAnyPlane(position, stone_path_planes) ||
        !IsPointOnAnyPlane(position, flat_ground_planes)
      ) {
        continue;
      }

      auto& flower = create(meshes.ground_flower);

      flower.position = position;
      flower.scale = tVec3f(250.f);
      flower.color = tVec3f(1.f, 0.1f, 0.1f);

      commit(flower);
    }
  });

  // @todo dev mode only
  {
    std::string message = "Generated " + std::to_string(objects(meshes.ground_flower).total_active) + " ground flower objects";

    console_log(message);
  }

  // @todo factor
  remove_all(meshes.tiny_ground_flower);

  // Clusters
  parallel_for_range(0, 7000, {
    tRNG rng(5678.f + float(__index));

    tVec3f center;
    center.x = rng.Random(-300000.f, 300000.f);
    center.y = -875.f;
    center.z = rng.Random(-300000.f, 300000.f);

    // 6 flowers per cluster
    // @todo avoid crashing at the limit! right now we only avoid
    // hitting the max because there isn't enough flat ground to
    // spawn flowers on
    for (int i = 0; i < 6; i++) {
      tVec3f position;
      position.x = center.x + rng.Random(-1000.f, 1000.f);
      position.y = center.y + rng.Random(-60.f, 60.f);
      position.z = center.z + rng.Random(-1000.f, 1000.f);

      if (
        IsPointOnAnyPlane(position, dirt_path_planes) ||
        IsPointOnAnyPlane(position, stone_path_planes) ||
        !IsPointOnAnyPlane(position, flat_ground_planes)
      ) {
        continue;
      }

      auto& flower = create(meshes.tiny_ground_flower);

      flower.position = position;
      flower.scale = tVec3f(100.f);
      flower.color = tVec3f(1.f);

      commit(flower);
    }
  });

  // @todo dev mode only
  {
    std::string message = "Generated " + std::to_string(objects(meshes.tiny_ground_flower).total_active) + " tiny ground flower objects";

    console_log(message);
  }
}

static void UpdateGroundFlowers(Tachyon* tachyon, State& state) {
  profile("UpdateGroundFlowers()");

  static const tVec3f offsets[] = {
    tVec3f(0, 0, 0),
    tVec3f(-500.f, 0, 450.f),
    tVec3f(350.f, 0, 400.f),
    tVec3f(600.f, 0, -350.f)
  };

  const tVec3f sprouting_color = tVec3f(0.1f, 0.6f, 0.1f);
  const tVec3f blossom_color = tVec3f(1.f, 0.1f, 0.1f);
  const tVec3f wilted_color = tVec3f(0.1f);

  auto& player_position = state.player_position;
  float base_time_progress = 0.5f * (state.astro_time - -500.f);

  // @todo factor
  const float lifetime = t_PI + t_HALF_PI;

  for (auto& flower : objects(state.meshes.ground_flower)) {
    if (abs(flower.position.x - player_position.x) > 15000.f) continue;
    if (abs(flower.position.z - player_position.z) > 15000.f) continue;

    float alpha_variation = fmodf(abs(flower.position.x + flower.position.z) * 0.1f, 10.f);
    float alpha = base_time_progress + alpha_variation;
    float life_cycles = alpha / lifetime;
    int life_cycle = (int)life_cycles + (int)abs(flower.position.x);

    tVec3f base_position = flower.position;

    flower.position = base_position + offsets[life_cycle % 4];

    UpdateBloomingFlower(flower, blossom_color, 200.f, alpha, lifetime);

    commit(flower);

    // Restore position after commit to avoid drift
    flower.position = base_position;
  }

  // @todo factor
  const float tiny_lifetime = t_TAU;

  for (auto& flower : objects(state.meshes.tiny_ground_flower)) {
    if (abs(flower.position.x - player_position.x) > 15000.f) continue;
    if (abs(flower.position.z - player_position.z) > 15000.f) continue;

    float alpha_variation = fmodf(abs(flower.position.x + flower.position.z) * 0.1f, 10.f);
    float alpha = base_time_progress + alpha_variation;
    float life_cycles = alpha / tiny_lifetime;
    int life_cycle = (int)life_cycles + (int)abs(flower.position.x);

    tVec3f base_position = flower.position;

    flower.position = base_position + offsets[life_cycle % 4];

    UpdateBloomingFlower(flower, tVec3f(1.f), 100.f, alpha, tiny_lifetime);

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
static void GenerateBushFlowers(Tachyon* tachyon, State& state) {
  remove_all(state.meshes.bush_flower);

  for (int i = 0; i < 500; i++) {
    commit(create(state.meshes.bush_flower));
  }
}

// @todo refactor with time_evolution.cpp -> GetLightColor()
static tVec3f GetBushFlowerBlossomColor(const float astro_time) {
  auto& periods = astro_time_periods;

  tVec3f present_color = tVec3f(1.f, 0.3f, 0.1f);
  tVec3f past_color = tVec3f(1.f, 0.8f, 0.2f);
  tVec3f distant_past_color = tVec3f(1.f, 0.8f, 1.f);

  if (astro_time < periods.present && astro_time >= periods.past) {
    float age_duration = periods.present - periods.past;
    float alpha = (astro_time - periods.past) / age_duration;

    return tVec3f::lerp(past_color, present_color, alpha);
  }

  if (astro_time < periods.past && astro_time >= periods.distant_past) {
    float age_duration = periods.past - periods.distant_past;
    float alpha = (astro_time - periods.distant_past) / age_duration;

    return tVec3f::lerp(distant_past_color, past_color, alpha);
  }

  if (astro_time < periods.distant_past) {
    // @todo lerp between very distant past -> distant past
    return distant_past_color;
  }

  return present_color;
}

// @todo just let FlowerBush.h handle this
static void UpdateBushFlowers(Tachyon* tachyon, State& state) {
  profile("UpdateBushFlowers()");

  const tVec3f blossom_color = GetBushFlowerBlossomColor(state.astro_time);
  const float spawn_radius = 1200.f;
  const float half_spawn_radius = spawn_radius * 0.5f;
  const float plant_lifetime = 100.f;
  const float flower_lifetime = 12.f;

  auto& player_position = state.player_position;
  float base_time_progress = 0.5f * (state.astro_time - -500.f);

  reset_instances(state.meshes.bush_flower);

  for_entities(state.flower_bushes) {
    auto& entity = state.flower_bushes[i];
    float end_time = entity.astro_start_time + plant_lifetime;

    if (state.astro_time < entity.astro_start_time + 20.f) continue;
    if (state.astro_time > end_time - 20.f) continue;

    float distance = tVec3f::distance(entity.visible_position, player_position);

    if (distance < 25000.f) {
      float entity_life_progress = GetLivingEntityProgress(state, entity, plant_lifetime);
      float flower_size = 300.f * sqrtf(sinf(entity_life_progress * t_PI));

      float vx = abs(entity.visible_position.x);
      float vz = abs(entity.visible_position.z);

      for (int i = 0; i < 3; i++) {
        auto& flower = use_instance(state.meshes.bush_flower);

        float offset_x = fmodf(vx + vx * 0.1f + 847.f * (float)i, spawn_radius) - half_spawn_radius;
        float offset_z = fmodf(vz + vz * 0.1f + 847.f * (float)i, spawn_radius) - half_spawn_radius;

        flower.position = entity.visible_position;
        flower.position.x += offset_x;
        flower.position.y += entity.visible_scale.y * 0.4f;
        flower.position.z += offset_z;

        // Give blossoms a bit of color variation based on position
        float flower_brightness = 1.f - 0.2f * fmodf(abs(flower.position.x), 1.f);
        tVec3f adjusted_blossom_color = blossom_color * flower_brightness;

        // Determine the progress of the flower
        // @todo rename
        float alpha_variation = fmodf(abs(flower.position.x + flower.position.z), 10.f);
        float alpha = base_time_progress + alpha_variation;

        UpdateBloomingFlower(flower, adjusted_blossom_color, flower_size, alpha, flower_lifetime);

        if (state.is_nighttime) {
          flower.color.rgba |= 0x0002;
        }

        flower.material = tVec4f(0.9f, 0, 0, 0.4f);

        commit(flower);
      }
    }
  }

  // @todo description
  auto& mesh = mesh(state.meshes.bush_flower);

  // @todo dev only
  {
    auto total_flowers = mesh(state.meshes.bush_flower).lod_1.instance_count;

    add_dev_label("  Total bush flowers: ", std::to_string(total_flowers));
  }
}

/**
 * ----------------------------
 * Dirt paths
 * ----------------------------
 */
static void GenerateDirtPaths(Tachyon* tachyon, State& state) {
  log_time("GenerateDirtPaths()");

  auto& meshes = state.meshes;

  remove_all(meshes.dirt_path);

  PathGeneration::GeneratePaths(tachyon, state, state.dirt_path_nodes, state.dirt_path_segments, meshes.dirt_path);

  // Rocks and debris pieces
  {
    remove_all(meshes.rock_dirt);

    for (uint16 i = 0; i < 400; i++) {
      create(meshes.rock_dirt);
    }
  }

  // @todo dev mode only
  {
    std::string message = "Generated " + std::to_string(objects(meshes.dirt_path).total_active) + " dirt path segments";

    console_log(message);
  }
}

static void UpdateDirtPaths(Tachyon* tachyon, State& state) {
  profile("UpdateDirtPaths()");

  auto& meshes = state.meshes;
  auto& player_position = state.player_position;

  const tVec3f solid_ground_color = tVec3f(0.3f, 0.5f, 0.1f);
  // @todo change by area/world position
  tVec3f path_color = tVec3f(1.f, 0.4f, 0.1f);
  const float distance_limit = 17000.f;

  uint16 total_rocks = 0;

  // @todo @optimize We can store an array of path segment "connections" based on entity index,
  // and just iterate over nearby nodes and then update their segments, rather than iterating
  // over all segments and doing player distance checks. In its current form this is still
  // quite fast, however.
  //
  // @todo move all this to DirtPathNode -> timeEvolve()
  for (auto& segment : state.dirt_path_segments) {
    auto& position = segment.base_position;

    if (abs(position.x - player_position.x) > distance_limit) continue;
    if (abs(position.z - player_position.z) > distance_limit) continue;

    auto& entity_a = state.dirt_path_nodes[segment.entity_index_a];
    auto& entity_b = state.dirt_path_nodes[segment.entity_index_b];
    auto& path = *get_live_object(segment.object);
    float astro_start_time = std::max(entity_a.astro_start_time, entity_b.astro_start_time);
    float astro_end_time = std::min(entity_a.astro_end_time, entity_b.astro_end_time);
    float age = state.astro_time - astro_start_time;
    float remaining_time = astro_end_time - state.astro_time;

    path.position = segment.base_position;
    path.position.y = -1470.f;
    path.scale = segment.base_scale;
    path.color = path_color;
    path.material = tVec4f(1.f, 0, 0, 0);

    // Reduce the size/conspicuousness of the path
    // as we approach its starting time
    if (age < 40.f && astro_start_time != 0.f) {
      const tVec3f ground_color = solid_ground_color;
      float alpha = age / 40.f;

      path.color = tVec3f::lerp(ground_color, path_color, alpha);
      path.position.y = Tachyon_Lerpf(-1500.f, path.position.y, alpha);
      path.scale.x *= alpha;
    }

    // Erode the path toward its end time
    if (remaining_time < 40.f && astro_end_time != 0.f) {
      const tVec3f ground_color = solid_ground_color;
      float alpha = remaining_time / 40.f;

      path.color = tVec3f::lerp(ground_color, path_color, alpha);
      path.position.y = Tachyon_Lerpf(-1500.f, path.position.y, alpha);
      path.scale.x *= alpha;
    }

    if (age < 0.f || (remaining_time < 0.f && astro_end_time != 0.f)) {
      // Path does not exist at the current astro time
      path.scale = tVec3f(0.f);
    } else {
      // Add little rocks to the edges of the path
      float size_variance = fmodf(abs(path.position.x), 170.f);

      auto& large_rock = objects(meshes.rock_dirt)[total_rocks++];
      auto& small_rock = objects(meshes.rock_dirt)[total_rocks++];

      large_rock.position = UnitObjectToWorldPosition(path, tVec3f(0.7f, 0, 0));
      large_rock.scale = tVec3f(10.f + size_variance);
      large_rock.rotation = path.rotation;
      large_rock.color = tVec3f(1.f, 0.4f, 0.2f);
      large_rock.material = tVec4f(1.f, 0, 0, 0.1f);

      small_rock.position = UnitObjectToWorldPosition(path, tVec3f(0.7f, 0, 0.8f));
      small_rock.scale = tVec3f(20.f + size_variance * 0.3f);
      small_rock.rotation = path.rotation;
      small_rock.color = tVec3f(0.8f, 0.4f, 0.2f);
      small_rock.material = tVec4f(1.f, 0, 0, 0.1f);

      commit(large_rock);
      commit(small_rock);
    }

    segment.plane = CollisionSystem::CreatePlane(path.position, path.scale, path.rotation);

    commit(path);
  }

  mesh(meshes.rock_dirt).lod_1.instance_count = total_rocks;
}

/**
 * ----------------------------
 * Stone paths
 * ----------------------------
 */
static void GenerateStonePaths(Tachyon* tachyon, State& state) {
  log_time("GenerateStonePaths()");

  auto& meshes = state.meshes;

  // Generate smaller path pieces
  remove_all(meshes.path_stone);

  for (uint16 i = 0; i < 1000; i++) {
    create(meshes.path_stone);
  }

  // Generate path segment objects
  remove_all(meshes.stone_path);

  PathGeneration::GeneratePaths(tachyon, state, state.stone_path_nodes, state.stone_path_segments, meshes.stone_path);

  // @todo dev mode only
  {
    std::string message = "Generated " + std::to_string(objects(state.meshes.stone_path).total_active) + " stone path segments";

    console_log(message);
  }
}

static void UpdateStonePaths(Tachyon* tachyon, State& state) {
  profile("UpdateStonePaths()");

  auto& meshes = state.meshes;
  auto& player_position = state.player_position;

  const tVec3f solid_ground_color = tVec3f(0.3f, 0.5f, 0.1f);
  tVec3f path_color = tVec3f(0.2f, 0.3f, 0.2f);

  const float distance_limit = 17000.f;

  uint16 total_path_stones = 0;

  Quaternion rotations[] = {
    Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), 0.f),
    Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), 0.1f),
    Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), 0.2f),
    Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), -0.1f),
    Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), -0.2f)
  };

  tVec3f positions[] = {
    // Center tile
    tVec3f(0.f, 0.f, 0.f),
    // Upper and lower center tiles
    tVec3f(+50.f, 0.f, +470.f),
    tVec3f(-50.f, 0.f, -470.f),
    // Corner tiles
    tVec3f(-560.f, 0.f, -300.f),
    tVec3f(+560.f, 0.f, -300.f),
    tVec3f(-560.f, 0.f, +300.f),
    tVec3f(+560.f, 0.f, +300.f)
  };

  float y_offsets[] = {
    15.f,
    20.f,
    30.f,
    10.f,
    25.f
  };

  for (auto& segment : state.stone_path_segments) {
    auto& position = segment.base_position;

    if (abs(position.x - player_position.x) > distance_limit) continue;
    if (abs(position.z - player_position.z) > distance_limit) continue;

    auto& entity_a = state.stone_path_nodes[segment.entity_index_a];
    auto& entity_b = state.stone_path_nodes[segment.entity_index_b];
    auto& path = *get_live_object(segment.object);
    float astro_start_time = std::max(entity_a.astro_start_time, entity_b.astro_start_time);
    float astro_end_time = std::min(entity_a.astro_end_time, entity_b.astro_end_time);
    float age = state.astro_time - astro_start_time;
    float remaining_time = astro_end_time - state.astro_time;

    path.position = segment.base_position;
    path.position.y = -1470.f;
    path.scale = segment.base_scale * 1.2f;
    path.color = path_color;
    path.material = tVec4f(1.f, 0, 0, 0);

    // Reduce the size/conspicuousness of the path
    // as we approach its starting time
    if (age < 30.f && astro_start_time != 0.f) {
      // @temporary
      const tVec3f ground_color = solid_ground_color;
      float alpha = age / 30.f;

      // @temporary
      path.color = tVec3f::lerp(ground_color, path_color, alpha);
      path.position.y = Tachyon_Lerpf(-1500.f, path.position.y, alpha);
      path.scale.x *= alpha;
    }

    // Erode the path toward its end time
    if (remaining_time < 30.f && astro_end_time != 0.f) {
      // @temporary
      const tVec3f ground_color = solid_ground_color;
      float alpha = remaining_time / 30.f;

      // @temporary
      path.color = tVec3f::lerp(ground_color, path_color, alpha);
      path.position.y = Tachyon_Lerpf(-1500.f, path.position.y, alpha);
      path.scale.x *= alpha;
    }

    if (
      age < 0.f ||
      (remaining_time < 0.f && astro_end_time != 0.f)
    ) {
      path.scale = tVec3f(0.f);
    }

    // Generate path stones
    // @todo make stones gradually appear/disappear with time
    // @todo weathering and aging with time
    {
      if (path.scale.x > 0.f) {
        float construction_alpha = age / 30.f;
        if (construction_alpha > 1.f) construction_alpha = 1.f;

        uint16 stones_per_segment = uint16(7.f * construction_alpha);

        uint16 start = total_path_stones;
        uint16 end = total_path_stones + stones_per_segment;
        int step = 0;

        tMat4f rotation_matrix = path.rotation.toMatrix4f();

        for (uint16 i = start; i < end; i++) {
          auto& stone = objects(meshes.path_stone)[i];
          // int iteration = int(abs(path.position.x + path.position.z) + step++);
          int y_iteration = int(abs(2.f * path.position.x + path.position.z));
          int rotation_iteration = int(abs(path.position.x + 2.f * path.position.z));

          stone.position = path.position + rotation_matrix * positions[step++];
          stone.position.y += y_offsets[y_iteration % 5];
          stone.scale = tVec3f(250.f);
          stone.rotation = path.rotation * rotations[rotation_iteration % 5];
          stone.color = tVec3f(0.4f);
          stone.material = tVec4f(0.1f, 0, 0, 0.2f);

          commit(stone);
        }

        total_path_stones = end;
      }
    }

    segment.plane = CollisionSystem::CreatePlane(path.position, path.scale, path.rotation);

    commit(path);
  }

  mesh(meshes.path_stone).lod_1.instance_count = total_path_stones;
}

/* ---------------------------- */

void ProceduralGeneration::RebuildSimpleProceduralObjects(Tachyon* tachyon, State& state) {
  GenerateDirtPaths(tachyon, state);
  GenerateStonePaths(tachyon, state);
  GenerateBushFlowers(tachyon, state);
}

void ProceduralGeneration::RebuildAllProceduralObjects(Tachyon* tachyon, State& state) {
  RebuildSimpleProceduralObjects(tachyon, state);

  GenerateTreeMushrooms(tachyon, state);
  GenerateGround1Plants(tachyon, state);
  GenerateSmallGrass(tachyon, state);

  GenerateGroundFlowers(tachyon, state);
}

void ProceduralGeneration::UpdateProceduralObjects(Tachyon* tachyon, State& state) {
  profile("UpdateProceduralObjects()");

  UpdateDirtPaths(tachyon, state);
  UpdateStonePaths(tachyon, state);

  UpdateTreeMushrooms(tachyon, state);
  UpdateGround1Plants(tachyon, state);
  UpdateSmallGrass(tachyon, state);

  UpdateGroundFlowers(tachyon, state);
  UpdateBushFlowers(tachyon, state);
}