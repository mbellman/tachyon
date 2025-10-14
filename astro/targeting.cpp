#include "astro/targeting.h"
#include "astro/entity_manager.h"

using namespace astro;

const static float target_distance_limit = 10000.f;

static inline void ResetEntityRecord(EntityRecord& record) {
  record.type = UNSPECIFIED;
  record.id = -1;
}

static EntityRecord GetClosestNonSelectedTarget(State& state) {
  float closest_distance = target_distance_limit;
  EntityRecord candidate;

  ResetEntityRecord(candidate);

  // @todo factor
  for_entities(state.low_guards) {
    auto& entity = state.low_guards[i];

    if (IsSameEntity(entity, state.target_entity)) {
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

    if (IsSameEntity(entity, state.target_entity)) {
      continue;
    }

    float distance = (state.player_position - entity.visible_position).magnitude();

    if (distance < closest_distance && entity.visible_scale.x != 0.f) {
      closest_distance = distance;

      candidate.id = entity.id;
      candidate.type = entity.type;
    }
  }

  return candidate;
}

static void HandleActiveTargetReticle(Tachyon* tachyon, State& state) {
  auto& reticle = objects(state.meshes.target_reticle)[0];
  auto& entity = *EntityManager::FindEntity(state, state.target_entity);
  float entity_distance = (state.player_position - entity.visible_position).magnitude();

  reticle.position = entity.visible_position;
  reticle.position.y += entity.visible_scale.y + 800.f;
  reticle.position.y += 100.f * sinf(t_TAU * tachyon->running_time);

  reticle.scale = tVec3f(400.f);
  reticle.color = tVec4f(1.f, 0.8f, 0.2f, 0.4f);
  reticle.rotation = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), 2.f * tachyon->running_time);

  if (entity_distance > 10000.f || entity.visible_scale.x == 0.f) {
    Targeting::DeselectCurrentTarget(tachyon, state);
  }

  commit(reticle);
}

static void HandleTargetPreviewReticle(Tachyon* tachyon, State& state) {
  auto& reticle = objects(state.meshes.target_reticle)[0];
  auto closest_target = GetClosestNonSelectedTarget(state);

  if (closest_target.type != UNSPECIFIED) {
    auto& entity = *EntityManager::FindEntity(state, closest_target);

    reticle.position = entity.visible_position;
    reticle.position.y += entity.visible_scale.y + 800.f;

    reticle.scale = tVec3f(250.f);
    reticle.color = tVec4f(1.f, 0.5f, 0.1f, 0.4f);
  } else {
    reticle.scale = tVec3f(0.f);
  }

  commit(reticle);
}

static void UpdateTargetReticle(Tachyon* tachyon, State& state) {
  if (state.has_target) {
    HandleActiveTargetReticle(tachyon, state);
  } else {
    HandleTargetPreviewReticle(tachyon, state);
  }
}

static void PickSpeakingEntity(State& state) {
  if (state.speaking_entity_record.type == UNSPECIFIED) {
    // Try to select a speaking entity among those within range
    auto closest_target = GetClosestNonSelectedTarget(state);

    Targeting::SetSpeakingEntity(state, closest_target);
  }
  else {
    auto& entity = *EntityManager::FindEntity(state, state.speaking_entity_record);
    float player_distance = (state.player_position - entity.visible_position).magnitude();

    if (player_distance > target_distance_limit) {
      // If the speaking entity goes out of range, reset our stored speaking entity
      ResetEntityRecord(state.speaking_entity_record);
    }
  }
}

void Targeting::HandleTargets(Tachyon* tachyon, State& state) {
  UpdateTargetReticle(tachyon, state);
  PickSpeakingEntity(state);
}

void Targeting::SetSpeakingEntity(State& state, EntityRecord& record) {
  state.speaking_entity_record = record;
}

void Targeting::SelectClosestAccessibleTarget(Tachyon* tachyon, State& state) {
  auto target_record = GetClosestNonSelectedTarget(state);

  if (target_record.type == UNSPECIFIED || target_record.id == -1) {
    return;
  }

  if (!state.has_target || !IsSameEntity(target_record, state.target_entity)) {
    state.has_target = true;
    state.target_start_time = tachyon->running_time;
    state.target_entity = target_record;

    Targeting::SetSpeakingEntity(state, target_record);
  }
}

void Targeting::SelectNextClosestAccessibleTarget(Tachyon* tachyon, State& state) {
  // Call SelectClosestAccessibleTarget() twice to have the second call
  // "skip" the initially-selected target and select the next-closest
  Targeting::SelectClosestAccessibleTarget(tachyon, state);
  Targeting::SelectClosestAccessibleTarget(tachyon, state);
}

void Targeting::DeselectCurrentTarget(Tachyon* tachyon, State& state) {
  state.has_target = false;

  if (IsSameEntity(state.target_entity, state.speaking_entity_record)) {
    ResetEntityRecord(state.speaking_entity_record);
  }

  ResetEntityRecord(state.target_entity);
}