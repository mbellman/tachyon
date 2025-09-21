#include "astro/collision_system.h"

using namespace astro;

static void HandleRadiusCollisions(State& state, const std::vector<GameEntity>& entities, float radius_scale) {
  auto& player_position = state.player_position;

  for_entities(entities) {
    auto& entity = entities[i];
    auto& position = entity.position;

    float radius = entity.visible_scale.x > entity.visible_scale.z
      ? 2.f * radius_scale * entity.visible_scale.x
      : 2.f * radius_scale * entity.visible_scale.z;

    // Skip small or invisible radii
    if (radius < 0.1f) continue;

    float dx = player_position.x - position.x;
    float dz = player_position.z - position.z;
    float distance = sqrtf(dx*dx + dz*dz);
    
    if (distance < radius) {
      float ratio = radius / distance;

      player_position.x = entity.position.x + dx * ratio;
      player_position.z = entity.position.z + dz * ratio;
    }
  }
}

void CollisionSystem::HandleCollisions(State& state) {
  HandleRadiusCollisions(state, state.shrubs, 1.f);
  HandleRadiusCollisions(state, state.oak_trees, 1.5f);

  // @todo handle collisions for other entity types
}