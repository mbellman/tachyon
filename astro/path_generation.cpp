#include <algorithm>
#include <functional>

#include "astro/path_generation.h"
#include "astro/collision_system.h"

using namespace astro;

static tVec3f HermiteInterpolate(const tVec3f& p0, const tVec3f& p1, const tVec3f& m0, const tVec3f& m1, float t) {
  float t2 = t * t;
  float t3 = t2 * t;

  float h00 = 2.0f * t3 - 3.0f * t2 + 1.0f;
  float h10 = t3 - 2.0f * t2 + t;
  float h01 = -2.0f * t3 + 3.0f * t2;
  float h11 = t3 - t2;

  return (p0 * h00) + (m0 * h10) + (p1 * h01) + (m1 * h11);
}

static void InitPathNetwork(PathNetwork& network, uint16 total_nodes) {
  network.total_nodes = total_nodes;
  network.nodes = new PathNode[total_nodes];
}

static void DestroyPathNetwork(PathNetwork& network) {
  delete[] network.nodes;
}

static bool DidWalkBetweenNodes(PathNode& a, PathNode& b) {
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
  float average_scale = (from_node.scale.magnitude() + to_node.scale.magnitude()) / 2.f;

  // Increase segments with distance and in inverse proportion to size
  int total_segments = int(distance / 1100.f) + int((1800.f - average_scale) / 150.f);

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

  if (DidWalkBetweenNodes(from_node, to_node)) {
    return;
  }

  // @todo No need for a loop here. We just grab the first next node
  // from to_node to use as a control point, but we don't need to iterate
  // over all next nodes.
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

    break;
  }

  SetAsWalkedBetween(from_node, to_node);

  for (uint16 i = 0; i < to_node.total_connections; i++) {
    auto& next_node = network.nodes[to_node.connections[i]];

    WalkPath(network, from_node, to_node, next_node, visitor);
  }
}

void PathGeneration::GeneratePaths(Tachyon* tachyon, State& state, const std::vector<GameEntity>& nodes, std::vector<PathSegment>& segments, const uint16 mesh_index) {
  // Generate the path network
  PathNetwork network;

  InitPathNetwork(network, (uint16)nodes.size());

  {
    for_entities(nodes) {
      auto& entity_a = nodes[i];
      float entity_a_scale = entity_a.scale.magnitude();
      uint16 index_a = i;

      auto& node = network.nodes[index_a];
      node.entity_index = index_a;
      node.position = entity_a.position;
      node.scale = entity_a.scale;

      for_entities(nodes) {
        auto& entity_b = nodes[i];
        uint16 index_b = i;

        if (IsSameEntity(entity_a, entity_b)) {
          continue;
        }

        float smallest_scale = std::min(entity_a_scale, entity_b.scale.magnitude());
        float distance = tVec3f::distance(entity_a.position, entity_b.position);

        float distance_threshold = smallest_scale * 5.75f;
        if (distance_threshold > 10000.f) distance_threshold = 10000.f;

        if (distance < distance_threshold) {
          if (node.total_connections < 4) {
            node.connections[node.total_connections++] = index_b;
          }
        }
      }
    }
  }

  // Generate path segments based on the path network
  {
    segments.clear();

    for (uint16 i = 0; i < network.total_nodes; i++) {
      auto& node = network.nodes[i];

      if (node.total_connections == 1) {
        auto& next_node = network.nodes[node.connections[0]];

        WalkPath(network, node, node, next_node, [tachyon, &state, &nodes, &segments, mesh_index](const tVec3f& position, const tVec3f& scale, const uint16 entity_index_a, const uint16 entity_index_b) {
          auto& path = create(mesh_index);

          auto& entity_a = nodes[entity_index_a];
          auto& entity_b = nodes[entity_index_b];

          // @temporary
          path.position = position;
          path.position.y = -1470.f;
          path.scale = scale;
          path.scale.y = 1.f;
          path.color = entity_a.tint;

          path.rotation = Quaternion::FromDirection((entity_b.position - entity_a.position).xz().unit(), tVec3f(0, 1.f, 0));

          // Flip some of the path segments 180 degrees to provide a bit of
          // visual variation.
          if ((int)(path.position.x * 0.01f) % 2 == 0) {
            path.rotation *= Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), t_PI);
          }

          commit(path);

          {
            PathSegment segment;
            segment.index = segments.size();
            segment.base_position = path.position;
            segment.base_scale = path.scale;
            segment.entity_index_a = entity_index_a;
            segment.entity_index_b = entity_index_b;
            segment.object = path;
            segment.plane = CollisionSystem::CreatePlane(path.position, path.scale, path.rotation);

            segments.push_back(segment);
          }
        });
      }
    }
  }

  DestroyPathNetwork(network);
}