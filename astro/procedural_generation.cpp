#include <functional>

#include "astro/procedural_generation.h"
#include "astro/collision_system.h"
#include "astro/entity_behaviors/behavior.h"

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

static std::vector<Plane> GetObjectPlanes(Tachyon* tachyon, uint16 mesh_index, const float scale_factor = 1.f) {
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
 * Grass
 * ----------------------------
 */
static void GenerateGrass(Tachyon* tachyon, State& state) {
  log_time("GenerateGrass()");

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

static void UpdateGrass(Tachyon* tachyon, State& state) {
  profile("UpdateGrass()");

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
static void GenerateSmallGrass(Tachyon* tachyon, State& state) {
  log_time("GenerateSmallGrass()");

  auto& meshes = state.meshes;

  const float chunk_width = 20000.f;
  const float chunk_height = 20000.f;

  static const float scales[] = {
    300.f,
    600.f,
    500.f,
    450.f
  };

  // @allocation
  auto flat_ground_planes = GetObjectPlanes(tachyon, meshes.flat_ground);
  auto ground_1_planes = GetObjectPlanes(tachyon, meshes.ground_1, 0.9f);
  auto dirt_path_planes = GetObjectPlanes(tachyon, meshes.p_dirt_path);

  // Reset objects/chunks/etc.
  {
    remove_all(meshes.small_grass);

    for (auto& chunk : state.grass_chunks) {
      chunk.grass_blades.clear();
    }

    state.grass_chunks.clear();
  }

  int32 total_chunks = 0;
  int32 total_blades = 0;

  for (int32 x = -10; x <= 10; x++) {
    for (int32 z = -10; z <= 10; z++) {
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

      // @allocation
      std::vector<Plane> local_ground_1_planes;
      std::vector<PathSegment> local_dirt_path_segments;

      for (auto& plane : ground_1_planes) {
        float distance = tVec3f::distance(plane.p1, chunk.center_position);

        if (distance < chunk_width * 1.2f) {
          local_ground_1_planes.push_back(plane);
        }
      }

      for (auto& segment : state.dirt_path_segments) {
        float distance = tVec3f::distance(segment.base_position, chunk.center_position);

        if (abs(segment.base_position.x - chunk.center_position.x) > chunk_width) continue;
        if (abs(segment.base_position.z - chunk.center_position.z) > chunk_height) continue;

        local_dirt_path_segments.push_back(segment);
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

        for (auto& segment : local_dirt_path_segments) {
          if (CollisionSystem::IsPointOnPlane(blade.position, segment.plane)) {
            blade.path_segment_index = segment.index;

            break;
          }
        }

        chunk.grass_blades.push_back(blade);

        total_blades++;
      }

      state.grass_chunks.push_back(chunk);

      total_chunks++;
    }
  }

  for (int i = 0; i < 50000; i++) {
    auto& grass = create(meshes.small_grass);

    grass.material = tVec4f(0.6f, 0, 0, 0.1f);

    commit(grass);
  }

  mesh(meshes.small_grass).lod_1.instance_count = 0;

  // @todo dev mode only
  {
    std::string message = "Generated " + std::to_string(total_chunks) + " grass chunks (" + std::to_string(total_blades) + " blades)";

    console_log(message);
  }
}

static void UpdateSmallGrass(Tachyon* tachyon, State& state) {
  profile("UpdateSmallGrass()");

  auto& meshes = state.meshes;

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
    450.f
  };

  tColor colors[] = {
    tVec4f(0.2f, 0.5f, 0.1f, 0.1f),
    tVec4f(0.3f, 0.6f, 0.1f, 0.1f),
    tVec4f(0.1f, 0.4f, 0.1f, 0.1f),
    tVec4f(0.1f, 0.5f, 0.1f, 0.1f)
  };

  const float growth_rate = 0.7f;

  auto& player_position = state.player_position;
  uint16 object_index = 0;

  for (auto& chunk : state.grass_chunks) {
    bool is_chunk_in_view = (
      abs(player_position.x - chunk.center_position.x) < 28000.f &&
      abs(player_position.z - chunk.center_position.z) < 25000.f
    );

    if (!is_chunk_in_view) continue;

    for (auto& blade : chunk.grass_blades) {
      float z_distance = blade.position.z - player_position.z;
      float x_limit = 15000.f + -z_distance * 0.5f;
      if (x_limit > 20000.f) x_limit = 20000.f;

      if (abs(blade.position.x - player_position.x) > x_limit) continue;
      if (z_distance > 8000.f) continue;
      if (z_distance < -18000.f) continue;

      if (blade.path_segment_index > -1) {
        auto& segment = state.dirt_path_segments[blade.path_segment_index];

        if (CollisionSystem::IsPointOnPlane(blade.position, segment.plane)) {
          continue;
        }
      }

      float alpha = state.astro_time + blade.position.x + blade.position.z;
      int iteration = (int)abs(growth_rate * alpha / t_TAU - 0.8f);
      float rotation_angle = float(iteration) * 1.3f;

      auto variation_index = iteration % 4;

      auto& grass = objects(meshes.small_grass)[object_index++];

      grass.position = blade.position;
      grass.position.y = -1500.f;

      grass.scale = blade.scale;

      grass.color = colors[variation_index];

      grass.rotation = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), rotation_angle);

      commit(grass);
    }
  }

  auto& mesh_record = mesh(meshes.small_grass);

  mesh_record.lod_1.instance_count = object_index;

  // @temporary
  add_dev_label("  Total small grass: ", std::to_string(object_index));
}

/**
 * ----------------------------
 * Ground flowers
 * ----------------------------
 */
static void GenerateGroundFlowers(Tachyon* tachyon, State& state) {
  log_time("GenerateGroundFlowers()");

  auto& meshes = state.meshes;

  remove_all(meshes.ground_flower);

  // @todo use path connection planes, not planes for each individual path segment
  auto dirt_path_planes = GetObjectPlanes(tachyon, meshes.p_dirt_path);
  auto flat_ground_planes = GetObjectPlanes(tachyon, meshes.flat_ground);
  // @todo check ground_1 planes

  tRNG rng(12345.f);

  // Clusters
  for (int i = 0; i < 2000; i++) {
    tVec3f center;
    // @todo increase range
    center.x = rng.Random(-150000.f, 150000.f);
    center.y = -1000.f;
    center.z = rng.Random(-150000.f, 150000.f);

    // 4 flowers per cluster
    for (int i = 0; i < 4; i++) {
      tVec3f position;
      position.x = center.x + rng.Random(-1500.f, 1500.f);
      position.y = center.y;
      position.z = center.z + rng.Random(-1500.f, 1500.f);

      if (
        IsPointOnAnyPlane(position, dirt_path_planes) ||
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
  }

  // @todo dev mode only
  {
    std::string message = "Generated " + std::to_string(objects(meshes.ground_flower).total_active) + " ground flower objects";

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

  const float lifetime = t_PI + t_HALF_PI;

  const tVec3f sprouting_color = tVec3f(0.1f, 0.6f, 0.1f);
  const tVec3f blossom_color = tVec3f(1.f, 0.1f, 0.1f);
  const tVec3f wilted_color = tVec3f(0.1f);

  auto& player_position = state.player_position;
  float base_time_progress = 0.5f * (state.astro_time - -500.f);

  for (auto& flower : objects(state.meshes.ground_flower)) {
    if (abs(flower.position.x - player_position.x) > 15000.f || abs(flower.position.z - player_position.z) > 15000.f) {
      continue;
    }

    float alpha_variation = fmodf(abs(flower.position.x + flower.position.z) * 0.1f, 10.f);
    float alpha = base_time_progress + alpha_variation;
    float life_cycles = alpha / lifetime;
    int life_cycle = (int)life_cycles + (int)abs(flower.position.x);

    tVec3f base_position = flower.position;

    flower.position = base_position + offsets[life_cycle % 5];

    UpdateBloomingFlower(flower, blossom_color, 250.f, alpha, lifetime);

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

  for (int i = 0; i < 200; i++) {
    commit(create(state.meshes.bush_flower));
  }
}

static void UpdateBushFlowers(Tachyon* tachyon, State& state) {
  profile("UpdateBushFlowers()");

  const tVec3f blossom_color = tVec3f(1.f, 0.6f, 0.1f);
  const float spawn_radius = 1200.f;
  const float half_spawn_radius = spawn_radius * 0.5f;
  const float lifetime = 12.f;

  auto& player_position = state.player_position;
  float base_time_progress = 0.5f * (state.astro_time - -500.f);
  uint16 index = 0;

  for_entities(state.flower_bushes) {
    auto& entity = state.flower_bushes[i];
    auto& position = entity.visible_position;

    if (entity.visible_scale.x < 500.f) {
      continue;
    }

    float distance = tVec3f::distance(entity.visible_position, player_position);

    if (distance < 18000.f) {
      float entity_life_progress = GetLivingEntityProgress(state, entity, 100.f);
      float flower_size = 400.f * sqrtf(sinf(entity_life_progress * t_PI));

      float ex = abs(entity.visible_position.x);
      float ez = abs(entity.visible_position.z);

      for (int i = 0; i < 3; i++) {
        auto& flower = objects(state.meshes.bush_flower)[index++];

        float offset_x = fmodf(10.f * ex + 723.f * (float)i, spawn_radius) - half_spawn_radius;
        float offset_z = fmodf(10.f * ez + 723.f * (float)i, spawn_radius) - half_spawn_radius;

        flower.position = entity.visible_position;
        flower.position.x += offset_x;
        flower.position.y += entity.visible_scale.y * 0.4f;
        flower.position.z += offset_z;

        float alpha_variation = fmodf(abs(flower.position.x + flower.position.z), 10.f);
        float alpha = base_time_progress + alpha_variation;

        UpdateBloomingFlower(flower, blossom_color, flower_size, alpha, lifetime);

        flower.material = tVec4f(0.5f, 0, 0, 0.2f);

        commit(flower);
      }
    }
  }

  // @todo description
  auto& mesh = mesh(state.meshes.bush_flower);

  mesh.lod_1.instance_count = index;
}

/**
 * ----------------------------
 * Dirt paths
 * @todo relocate foundational pathing code
 * ----------------------------
 */
tVec3f HermiteInterpolate(const tVec3f& p0, const tVec3f& p1, const tVec3f& m0, const tVec3f& m1, float t) {
  float t2 = t * t;
  float t3 = t2 * t;

  float h00 = 2.0f * t3 - 3.0f * t2 + 1.0f;
  float h10 = t3 - 2.0f * t2 + t;
  float h01 = -2.0f * t3 + 3.0f * t2;
  float h11 = t3 - t2;

  return (p0 * h00) + (m0 * h10) + (p1 * h01) + (m1 * h11);
}

// @todo move elsewhere
struct PathNode {
  int32 entity_index = -1;
  tVec3f position;
  tVec3f scale;

  uint16 connections[4] = { 0, 0, 0, 0 };
  uint16 total_connections = 0;

  int32 connections_walked[4] = { -1, -1, -1, -1 };
  uint16 total_connections_walked = 0;
};

struct PathNetwork {
  PathNode* nodes = nullptr;
  uint16 total_nodes = 0;
};

static void InitPathNetwork(PathNetwork& network, uint16 total_nodes) {
  network.total_nodes = total_nodes;
  network.nodes = new PathNode[total_nodes];
}

static void DestroyPathNetwork(PathNetwork& network) {
  delete[] network.nodes;
}

static bool DidWalkBetweenNodes(const PathNetwork& network, PathNode& a, PathNode& b) {
  for (uint16 i = 0; i < a.total_connections_walked; i++) {
    if (a.connections_walked[i] == b.entity_index) {
      return true;
    }
  }

  for (uint16 i = 0; i < b.total_connections_walked; i++) {
    if (b.connections_walked[i] == a.entity_index) {
      return true;
    }
  }

  return false;
}

static void SetAsWalkedBetween(PathNode& from_node, PathNode& to_node) {
  from_node.connections_walked[from_node.total_connections_walked++] = to_node.entity_index;
}

// @todo fix issues with path start/end points
using PathVisitor = std::function<void(const tVec3f&, const tVec3f&, const uint16, const uint16)>;

static void WalkPath(const PathNetwork& network, PathNode& previous_node, PathNode& from_node, PathNode& to_node, const PathVisitor& visitor) {
  float distance = (from_node.position - to_node.position).magnitude();
  int total_segments = int(distance / 1100.f);

  if (from_node.total_connections == 1) {
    for (int i = 0; i < total_segments; i++) {
      float alpha = float(i) / float(total_segments);

      // @todo still not quite right
      tVec3f delta = to_node.position - from_node.position;
      float length = delta.magnitude();
      tVec3f direction = delta / length;
      tVec3f m0 = tVec3f::cross(direction, tVec3f(0, 1.f, 0)) * length;
      tVec3f m1 = m0 * 0.5f;
      tVec3f position = HermiteInterpolate(from_node.position, to_node.position, m0, m1, alpha);

      tVec3f scale = tVec3f::lerp(from_node.scale, to_node.scale, alpha);

      visitor(position, scale, from_node.entity_index, to_node.entity_index);
    }
  }

  if (DidWalkBetweenNodes(network, from_node, to_node)) {
    return;
  }

  for (uint16 i = 0; i < to_node.total_connections; i++) {
    auto& next_node = network.nodes[to_node.connections[i]];

    if (next_node.entity_index == from_node.entity_index) continue;

    for (int i = 0; i < total_segments; i++) {
      float alpha = float(i) / float(total_segments);

      tVec3f m0 = (to_node.position - previous_node.position) * 0.5f;
      tVec3f m1 = (next_node.position - to_node.position) * 0.5f;
      tVec3f position = HermiteInterpolate(from_node.position, to_node.position, m0, m1, alpha);

      tVec3f scale = tVec3f::lerp(from_node.scale, to_node.scale, alpha);

      visitor(position, scale, from_node.entity_index, to_node.entity_index);
    }
  }

  SetAsWalkedBetween(from_node, to_node);

  for (uint16 i = 0; i < to_node.total_connections; i++) {
    to_node.connections_walked[i] = true;

    auto& next_node = network.nodes[to_node.connections[i]];

    WalkPath(network, from_node, to_node, next_node, visitor);
  }
}

static void GenerateDirtPaths(Tachyon* tachyon, State& state) {
  log_time("GenerateDirtPaths()");

  // @temporary
  // @todo remove regular dirt path entities
  objects(state.meshes.dirt_path_placeholder).disabled = true;
  objects(state.meshes.dirt_path).disabled = true;

  remove_all(state.meshes.p_dirt_path);

  // Generate the path network
  PathNetwork network;

  InitPathNetwork(network, (uint16)state.dirt_path_nodes.size());

  {
    network.total_nodes = (uint16)state.dirt_path_nodes.size();
    network.nodes = new PathNode[network.total_nodes];

    for_entities(state.dirt_path_nodes) {
      auto& entity_a = state.dirt_path_nodes[i];
      uint16 index_a = i;

      auto& node = network.nodes[index_a];
      node.position = entity_a.position;
      node.scale = entity_a.scale;
      node.entity_index = i;

      for_entities(state.dirt_path_nodes) {
        auto& entity_b = state.dirt_path_nodes[i];
        uint16 index_b = i;

        if (IsSameEntity(entity_a, entity_b)) {
          continue;
        }

        tVec3f connection = entity_a.position - entity_b.position;
        float distance = connection.magnitude();

        if (distance < 10000.f) {
          if (node.total_connections < 4) {
            node.connections[node.total_connections++] = index_b;
          }
        }
      }
    }
  }

  // Generate dirt path segments based on the path network
  // @todo investigate why paths don't start and end at nodes
  {
    state.dirt_path_segments.clear();

    for (uint16 i = 0; i < network.total_nodes; i++) {
      auto& node = network.nodes[i];

      if (node.total_connections == 1) {
        auto& next_node = network.nodes[node.connections[0]];

        WalkPath(network, node, node, next_node, [tachyon, &state](const tVec3f& position, const tVec3f& scale, const uint16 entity_index_a, const uint16 entity_index_b) {
          auto& path = create(state.meshes.p_dirt_path);

          auto& entity_a = state.dirt_path_nodes[entity_index_a];
          auto& entity_b = state.dirt_path_nodes[entity_index_b];

          // @temporary
          path.position = position;
          path.position.y = -1470.f;
          path.scale = scale;
          path.scale.y = 1.f;
          path.color = entity_a.tint;

          path.rotation = Quaternion::FromDirection((entity_b.position - entity_a.position).xz().unit(), tVec3f(0, 1.f, 0));

          // Flip some of the path segments 180 degrees to provide a bit of
          // visual variation.
          //
          // @todo see if we need to do this as we improve path visual fidelity
          if ((int)(path.position.x * 0.01f) % 2 == 0) {
            path.rotation *= Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), t_PI);
          }

          commit(path);

          {
            PathSegment segment;
            segment.index = state.dirt_path_segments.size();
            segment.base_position = path.position;
            segment.base_scale = path.scale;
            segment.entity_index_a = entity_index_a;
            segment.entity_index_b = entity_index_b;
            segment.object = path;
            segment.plane = CollisionSystem::CreatePlane(path.position, path.scale, path.rotation);

            state.dirt_path_segments.push_back(segment);
          }
        });
      }
    }
  }

  DestroyPathNetwork(network);

  // @todo dev mode only
  {
    std::string message = "Generated " + std::to_string(objects(state.meshes.p_dirt_path).total_active) + " dirt path objects";

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

    // Reduce the size/conspicuousness of the path
    // as we approach its starting time
    if (age < 40.f && astro_start_time != 0.f) {
      // @temporary
      const tVec3f ground_color = solid_ground_color;
      float alpha = age / 40.f;

      path.color = tVec3f::lerp(ground_color, path_color, alpha);
      path.position.y = Tachyon_Lerpf(-1500.f, path.position.y, alpha);
      path.scale.x *= alpha;
    }

    // Erode the path toward its end time
    if (remaining_time < 40.f && astro_end_time != 0.f) {
      // @temporary
      const tVec3f ground_color = solid_ground_color;
      float alpha = remaining_time / 40.f;

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

    segment.plane = CollisionSystem::CreatePlane(path.position, path.scale, path.rotation);

    commit(path);
  }
}

/* ---------------------------- */

void ProceduralGeneration::RebuildProceduralObjects(Tachyon* tachyon, State& state) {
  GenerateDirtPaths(tachyon, state);

  // @todo refactor these two
  GenerateGrass(tachyon, state);
  GenerateSmallGrass(tachyon, state);

  GenerateGroundFlowers(tachyon, state);
  GenerateBushFlowers(tachyon, state);

}

void ProceduralGeneration::UpdateProceduralObjects(Tachyon* tachyon, State& state) {
  profile("UpdateProceduralObjects()");

  UpdateDirtPaths(tachyon, state);

  // @todo refactor these two
  UpdateGrass(tachyon, state);
  UpdateSmallGrass(tachyon, state);

  UpdateGroundFlowers(tachyon, state);
  UpdateBushFlowers(tachyon, state);
}