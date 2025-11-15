#include "astro/targeting.h"
#include "astro/entity_manager.h"
#include "astro/entity_behaviors/behavior.h"

using namespace astro;

const static float target_distance_limit = 10000.f;

static int32 FindRecordIndex(std::vector<EntityRecord>& records, EntityRecord& record) {
  for (size_t i = 0; i < records.size(); i++) {
    auto& compared = records[i];

    if (IsSameEntity(record, compared)) {
      return (int32)i;
    }
  }

  return -1;
}

static inline void ResetEntityRecord(EntityRecord& record) {
  record.type = UNSPECIFIED;
  record.id = -1;
}

// @todo @optimize this function is kind of dumb now.
// the lookup to resolve entities is unnecessary if we
// precompute this stuff in TrackTargetableEntities()
// and use that information where needed.
static EntityRecord GetClosestNonSelectedTarget(State& state) {
  float closest_distance = target_distance_limit;
  EntityRecord candidate;

  ResetEntityRecord(candidate);

  for (auto& record : state.targetable_entities) {
    if (IsSameEntity(record, state.target_entity)) {
      continue;
    }

    auto& entity = *EntityManager::FindEntity(state, record);
    float distance = tVec3f::distance(state.player_position, entity.visible_position);

    if (distance < closest_distance && entity.visible_scale.x != 0.f) {
      closest_distance = distance;
      candidate = record;
    }
  }

  return candidate;
}

static void TrackTargetableEntities(State& state) {
  state.targetable_entities.clear();

  // @todo factor
  for_entities(state.low_guards) {
    auto& entity = state.low_guards[i];

    if (!IsDuringActiveTime(entity, state)) continue;

    float player_distance = tVec3f::distance(entity.visible_position, state.player_position);

    if (player_distance < target_distance_limit) {
      state.targetable_entities.push_back(GetRecord(entity));
    }
  }

  // @todo factor
  for_entities(state.bandits) {
    auto& entity = state.bandits[i];

    if (!IsDuringActiveTime(entity, state)) continue;

    float player_distance = tVec3f::distance(entity.visible_position, state.player_position);

    if (player_distance < target_distance_limit) {
      state.targetable_entities.push_back(GetRecord(entity));
    }
  }
}

static void HandleActiveTargetReticle(Tachyon* tachyon, State& state) {
  auto& reticle = objects(state.meshes.target_reticle)[0];
  auto& entity = *EntityManager::FindEntity(state, state.target_entity);
  float entity_distance = (state.player_position - entity.visible_position).magnitude();
  float time = tachyon->scene.scene_time;

  reticle.position = entity.visible_position;
  reticle.position.y += entity.visible_scale.y + 1200.f;
  reticle.position.y += 100.f * sinf(t_TAU * time);

  reticle.scale = tVec3f(400.f);
  reticle.color = tVec4f(1.f, 0.8f, 0.2f, 0.4f);
  reticle.rotation = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), -2.f * time);

  if (entity_distance > 10000.f || entity.visible_scale.x == 0.f) {
    Targeting::DeselectCurrentTarget(tachyon, state);
  }

  commit(reticle);
}

static void HandlePreviewTargetReticle(Tachyon* tachyon, State& state) {
  auto& reticle = objects(state.meshes.target_reticle)[0];
  auto closest_target = GetClosestNonSelectedTarget(state);

  if (closest_target.type != UNSPECIFIED) {
    auto& entity = *EntityManager::FindEntity(state, closest_target);
    float time = tachyon->scene.scene_time;

    reticle.position = entity.visible_position;
    reticle.position.y += entity.visible_scale.y + 1200.f;
    reticle.position.y += 150.f * sinf(t_TAU * tachyon->running_time);

    reticle.scale = tVec3f(250.f);
    reticle.color = tVec4f(0.7f, 0.2f, 0.1f, 0.8f);

    reticle.rotation = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), -2.f * time);
  } else {
    reticle.scale = tVec3f(0.f);
  }

  commit(reticle);
}

static void UpdateTargetReticle(Tachyon* tachyon, State& state) {
  if (state.has_target) {
    HandleActiveTargetReticle(tachyon, state);
  } else {
    HandlePreviewTargetReticle(tachyon, state);
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

static inline bool ShouldSelectTarget(State& state, EntityRecord& record) {
  return !state.has_target || !IsSameEntity(record, state.target_entity);
}

static void SelectTarget(Tachyon* tachyon, State& state, EntityRecord& target) {
  state.has_target = true;
  state.target_start_time = tachyon->running_time;
  state.target_entity = target;

  Targeting::SetSpeakingEntity(state, target);
}

void Targeting::HandleTargets(Tachyon* tachyon, State& state) {
  TrackTargetableEntities(state);
  UpdateTargetReticle(tachyon, state);
  PickSpeakingEntity(state);
}

void Targeting::SetSpeakingEntity(State& state, EntityRecord& record) {
  state.speaking_entity_record = record;
}

void Targeting::SelectNextAccessibleTarget(Tachyon* tachyon, State& state) {
  EntityRecord new_target;

  ResetEntityRecord(new_target);

  if (state.has_target) {
    auto index = FindRecordIndex(state.targetable_entities, state.target_entity);

    if (index > -1) {
      if (index < state.targetable_entities.size() - 1) {
        new_target = state.targetable_entities[index + 1];
      } else {
        new_target = state.targetable_entities[0];
      }
    }
  } else {
    new_target = GetClosestNonSelectedTarget(state);
  }

  if (new_target.type == UNSPECIFIED || new_target.id == -1) {
    return;
  }

  if (ShouldSelectTarget(state, new_target)) {
    SelectTarget(tachyon, state, new_target);
  }
}

void Targeting::SelectPreviousAccessibleTarget(Tachyon* tachyon, State& state) {
  EntityRecord new_target;

  ResetEntityRecord(new_target);

  if (state.has_target) {
    auto index = FindRecordIndex(state.targetable_entities, state.target_entity);

    if (index > -1) {
      if (index > 0) {
        new_target = state.targetable_entities[index - 1];
      } else {
        new_target = state.targetable_entities.back();
      }
    }
  } else {
    new_target = GetClosestNonSelectedTarget(state);
  }

  if (new_target.type == UNSPECIFIED || new_target.id == -1) {
    return;
  }

  if (ShouldSelectTarget(state, new_target)) {
    SelectTarget(tachyon, state, new_target);
  }
}

void Targeting::DeselectCurrentTarget(Tachyon* tachyon, State& state) {
  state.has_target = false;

  if (IsSameEntity(state.target_entity, state.speaking_entity_record)) {
    ResetEntityRecord(state.speaking_entity_record);
  }

  ResetEntityRecord(state.target_entity);
}

bool Targeting::IsInCombatWithAnyTarget(State& state) {
  for (auto& record : state.targetable_entities) {
    auto& entity = *EntityManager::FindEntity(state, record);

    if (entity.enemy_state.mood == ENEMY_AGITATED) {
      return true;
    }
  }

  return false;
}