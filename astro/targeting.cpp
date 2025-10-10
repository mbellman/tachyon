#include "astro/targeting.h"
#include "astro/entity_manager.h"

using namespace astro;

static inline bool IsSame(GameEntity& entity, EntityRecord& record) {
  return entity.type == record.type && entity.id == record.id;
}

static inline bool IsSame(EntityRecord& record_a, EntityRecord& record_b) {
  return record_a.type == record_b.type && record_a.id == record_b.id;
}

static EntityRecord GetClosestNonSelectedTarget(State& state) {
  const float target_distance_limit = 10000.f;
  float closest_distance = target_distance_limit;
  EntityRecord candidate;

  // @todo factor
  for_entities(state.low_guards) {
    auto& entity = state.low_guards[i];

    if (IsSame(entity, state.target_entity)) {
      continue;
    }

    float distance = (state.player_position - entity.visible_position).magnitude();

    if (distance < closest_distance && entity.visible_scale.x != 0.f) {
      closest_distance = distance;

      candidate.id = entity.id;
      candidate.type = entity.type;
    }
  }

  // @todo factor
  for_entities(state.bandits) {
    auto& entity = state.bandits[i];

    if (IsSame(entity, state.target_entity)) {
      continue;
    }

    float distance = (state.player_position - entity.visible_position).magnitude();

    if (distance < closest_distance && entity.visible_scale.x != 0.f) {
      closest_distance = distance;

      candidate.id = entity.id;
      candidate.type = entity.type;
    }
  }

  if (closest_distance == target_distance_limit) {
    candidate.type = UNSPECIFIED;
    candidate.id = -1;
  }

  return candidate;
}

void Targeting::HandleCurrentTarget(Tachyon* tachyon, State& state) {
  auto& reticle = objects(state.meshes.target_reticle)[0];

  if (state.has_target) {
    auto& entity = *EntityManager::FindEntity(state, state.target_entity);
    float entity_distance = (state.player_position - entity.visible_position).magnitude();

    reticle.position = entity.visible_position;
    reticle.position.y += entity.visible_scale.y + 800.f;

    reticle.scale = tVec3f(300.f);
    reticle.color = tVec4f(1.f, 0.8f, 0.2f, 0.4f);
    reticle.rotation = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), 2.f * tachyon->running_time);

    if (entity_distance > 10000.f || entity.visible_scale.x == 0.f) {
      Targeting::DeselectCurrentTarget(tachyon, state);
    }
  } else {
    reticle.scale = tVec3f(0.f);
  }

  commit(reticle);
}

void Targeting::SelectNearestAccessibleTarget(Tachyon* tachyon, State& state) {
  auto target_record = GetClosestNonSelectedTarget(state);

  if (target_record.type == UNSPECIFIED || target_record.id == -1) {
    return;
  }

  if (!state.has_target || !IsSame(target_record, state.target_entity)) {
    state.has_target = true;
    state.target_start_time = tachyon->running_time;
    state.target_entity = target_record;
  }
}

void Targeting::DeselectCurrentTarget(Tachyon* tachyon, State& state) {
  state.has_target = false;

  state.target_entity.type = UNSPECIFIED;
  state.target_entity.id = -1;
}