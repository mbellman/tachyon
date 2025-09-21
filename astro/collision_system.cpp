#include "astro/collision_system.h"

using namespace astro;

constexpr static float player_radius = 600.f;

static inline void ResolveSingleRadiusCollision(State& state, const tVec3f& position, const tVec3f& scale, float radius_scale) {
  float radius = scale.x > scale.z
    ? radius_scale * scale.x
    : radius_scale * scale.z;

  // Skip small or invisible radii
  if (radius < 0.1f) return;

  float dx = state.player_position.x - position.x;
  float dz = state.player_position.z - position.z;
  float distance = sqrtf(dx*dx + dz*dz) - player_radius;

  // Prevent excessively small or negative distances
  if (distance < 100.f) distance = 100.f;

  if (distance < radius) {
    float ratio = radius / distance;

    // Ensure the ratio is kept small to avoid large
    // instantaneous displacements out of the radius
    if (ratio > 1.1f) ratio = 1.1f;

    state.player_position.x = position.x + dx * ratio;
    state.player_position.z = position.z + dz * ratio;
  }
}

void CollisionSystem::HandleCollisions(Tachyon* tachyon, State& state) {
  for_entities(state.shrubs) {
    auto& entity = state.shrubs[i];

    ResolveSingleRadiusCollision(state, entity.position, entity.visible_scale, 1.5f);
  }

  for_entities(state.oak_trees) {
    auto& entity = state.oak_trees[i];

    ResolveSingleRadiusCollision(state, entity.position, entity.visible_scale, 1.5f);
  }

  for (auto& rock : objects(state.meshes.rock_1)) {
    ResolveSingleRadiusCollision(state, rock.position, rock.scale, 1.f);
  }
}