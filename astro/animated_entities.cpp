#include "astro/animated_entities.h"
#include "astro/animation.h"
#include "astro/entity_behaviors/behavior.h"

using namespace astro;

struct ActiveAnimation {
  tSkeletonAnimation* animation;
  float speed;
  bool immediate = false;
};

bool IsEnemyHit(Tachyon* tachyon, GameEntity& entity) {
  auto& enemy = entity.enemy_state;

  return (
    time_since(enemy.last_break_time) < 1.f ||
    time_since(enemy.last_damage_time) < 1.f
  );
}

static void UpdateAnimation(tSkinnedMeshAnimation& animation, const float speed, const float dt) {
  Animation::AccumulateTime(animation, speed, 3.f, dt);
  Animation::UpdatePose(animation, BLEND_LINEAR);
  Animation::UpdateBoneMatrices(animation);
}

static void SyncSkinnedMesh(tSkinnedMesh& mesh, GameEntity& entity, tSkinnedMeshAnimation& animation) {
  mesh.position = entity.visible_position;
  mesh.rotation = entity.visible_rotation;
  mesh.scale = tVec3f(1500.f);
  mesh.shadow_cascade_ceiling = 1;
  mesh.disabled = false;
  mesh.current_pose = &animation.active_pose;
}

static void AttachToHead(tObject& object, ReservedSkinnedMesh& skin, const tVec3f& body_position, const Quaternion& body_rotation) {
  auto& head_bone = skin.animation.active_pose.bones[0];
  tVec3f scale = tVec3f(1500.f);

  tVec3f head_offset;
  head_offset += head_bone.translation * scale;
  head_offset += head_bone.rotation.toMatrix4f() * tVec3f(0, scale.y * 0.35f, 0);

  object.position = body_position + body_rotation.toMatrix4f() * head_offset;
  object.rotation = body_rotation * head_bone.rotation;
  object.scale = scale;
}

static void AttachToRightArm(tObject& object, ReservedSkinnedMesh& skin, const tVec3f& body_position, const Quaternion& body_rotation) {
  auto& bone = skin.animation.active_pose.bones[5];
  tVec3f scale = tVec3f(1500.f);

  tVec3f offset;
  offset += bone.translation * scale;

  object.position = body_position + body_rotation.toMatrix4f() * offset;
  object.rotation = body_rotation * bone.rotation;
  object.scale = scale;
}

static void AttachToLeftArm(tObject& object, ReservedSkinnedMesh& skin, const tVec3f& body_position, const Quaternion& body_rotation) {
  auto& bone = skin.animation.active_pose.bones[2];
  tVec3f scale = tVec3f(1500.f);

  tVec3f offset;
  offset += bone.translation * scale;

  object.position = body_position + body_rotation.toMatrix4f() * offset;
  object.rotation = body_rotation * bone.rotation;
  object.scale = scale;
}

/**
 * -------------
 * Lesser guards
 * -------------
 */
static ActiveAnimation GetLesserGuardActiveAnimation(Tachyon* tachyon, State& state, GameEntity& entity) {
  auto& enemy = entity.enemy_state;

  if (time_since(enemy.last_damage_time) < 1.f) {
    return { &state.animations.person_hit_front, 5.f, true };
  }
  else if (time_since(enemy.last_break_time) < 1.f) {
    // @todo break animation
    return { &state.animations.person_hit_front, 5.f, true };
  }
  else if (enemy.speed > 2500.f) {
    return { &state.animations.player_run, 10.f };
  }
  else if (enemy.speed > 500.f) {
    return { &state.animations.player_walk, 10.f };
  }
  else {
    return { &state.animations.person_idle, 1.f };
  }
}

static void HandleLesserGuardAnimations(Tachyon* tachyon, State& state, int32& usage_counter) {
  auto& meshes = state.meshes;
  auto& animations = state.animations;

  reset_instances(meshes.lesser_helmet);
  reset_instances(meshes.lesser_vambrace);

  if (state.enemies_disabled) {
    return;
  }

  for_entities(state.lesser_guards) {
    auto& entity = state.lesser_guards[i];

    if (abs(state.player_position.x - entity.visible_position.x) > 15000.f) continue;
    if (abs(state.player_position.z - entity.visible_position.z) > 15000.f) continue;
    if (!IsDuringActiveTime(entity, state)) continue;

    auto& skin = state.person_skinned_meshes[usage_counter++];
    // auto& person = skinned_mesh(skin.mesh_index);
    auto& shirt = skinned_mesh(skin.shirt_mesh_index);

    if (skin.animation.current_animation == nullptr) {
      skin.animation.current_animation = &animations.player_run;
    }

    auto active_animation = GetLesserGuardActiveAnimation(tachyon, state, entity);

    if (active_animation.immediate) {
      Animation::StartNextAnimation(skin.animation, active_animation.animation);
    } else {
      Animation::AwaitNextAnimation(skin.animation, active_animation.animation);
    }

    UpdateAnimation(skin.animation, active_animation.speed, state.dt);
    // SyncSkinnedMesh(person, entity, skin.animation);
    SyncSkinnedMesh(shirt, entity, skin.animation);

    shirt.color = tVec3f(0.5f, 0.8f, 0.7f);
    shirt.material = tVec4f(0.8f, 0, 0, 0.4f);

    tVec3f body_position = entity.visible_position;
    Quaternion body_rotation = entity.visible_rotation;

    // @todo factor
    if (entity.enemy_state.last_death_time != 0.f) {
      float death_alpha = 2.f * time_since(entity.enemy_state.last_death_time);
      if (death_alpha > 1.f) death_alpha = 1.f;

      Quaternion death_rotation = body_rotation * (
        Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), -t_HALF_PI)
      );

      body_rotation = Quaternion::slerp(body_rotation, death_rotation, death_alpha);

      body_position = tVec3f::lerp(
        body_position,
        body_position - tVec3f(0, 1200.f, 0),
        death_alpha
      );
    }

    shirt.position = body_position;
    shirt.rotation = body_rotation;

    // commit(person);
    commit(shirt);

    // Helmet
    {
      auto& helmet = use_instance(meshes.lesser_helmet);

      helmet.color = tVec3f(0.7f, 0.4f, 0.1f);
      helmet.material = tVec4f(0.5f, 0, 0, 0.2f);

      AttachToHead(helmet, skin, body_position, body_rotation);

      commit(helmet);
    }

    // Vambraces
    {
      auto& left_vambrace = use_instance(meshes.lesser_vambrace);
      auto& right_vambrace = use_instance(meshes.lesser_vambrace);

      left_vambrace.color = tVec3f(0.7f, 0.4f, 0.1f);
      left_vambrace.material = tVec4f(0.5f, 0, 0, 0.2f);

      right_vambrace.color = tVec3f(0.7f, 0.4f, 0.1f);
      right_vambrace.material = tVec4f(0.5f, 0, 0, 0.2f);

      AttachToLeftArm(left_vambrace, skin, body_position, body_rotation);
      AttachToRightArm(right_vambrace, skin, body_position, body_rotation);

      commit(left_vambrace);
      commit(right_vambrace);
    }
  }
}

/**
 * ----------
 * Low guards
 * ----------
 */
static void HandleLowGuardAnimations(Tachyon* tachyon, State& state, int32& usage_counter) {
  auto& meshes = state.meshes;
  auto& animations = state.animations;

  if (state.enemies_disabled) {
    return;
  }

  for_entities(state.low_guards) {
    auto& entity = state.low_guards[i];

    if (abs(state.player_position.x - entity.visible_position.x) > 15000.f) continue;
    if (abs(state.player_position.z - entity.visible_position.z) > 25000.f) continue;
    if (!IsDuringActiveTime(entity, state)) continue;

    auto& skin = state.person_skinned_meshes[usage_counter++];
    auto& person = skinned_mesh(skin.mesh_index);

    if (skin.animation.current_animation == nullptr) {
      skin.animation.current_animation = &animations.player_run;
    }

    // @temporary
    auto active_animation = GetLesserGuardActiveAnimation(tachyon, state, entity);

    if (active_animation.immediate) {
      Animation::StartNextAnimation(skin.animation, active_animation.animation);
    } else {
      Animation::AwaitNextAnimation(skin.animation, active_animation.animation);
    }

    UpdateAnimation(skin.animation, active_animation.speed, state.dt);
    SyncSkinnedMesh(person, entity, skin.animation);

    commit(person);

    // Armor parts
    // @todo
  }
}

/**
 * ----
 * NPCs
 * ----
 */
static void HandleNPCAnimations(Tachyon* tachyon, State& state, int32& usage_counter) {
  auto& meshes = state.meshes;
  auto& animations = state.animations;

  for_entities(state.npcs) {
    auto& entity = state.npcs[i];

    if (abs(state.player_position.x - entity.visible_position.x) > 15000.f) continue;
    if (abs(state.player_position.z - entity.visible_position.z) > 15000.f) continue;
    if (!IsDuringActiveTime(entity, state)) continue;

    auto& skin = state.person_skinned_meshes[usage_counter++];
    auto& person = skinned_mesh(skin.mesh_index);

    if (skin.animation.current_animation == nullptr) {
      skin.animation.current_animation = &animations.person_idle;
    }

    float animation_speed;

    // @todo improve determinant for current talking npc
    if (state.current_dialogue_set == entity.unique_name) {
      Animation::SetNextAnimation(skin.animation, &animations.person_talking);

      animation_speed = 1.75f;
    } else {
      Animation::SetNextAnimation(skin.animation, &animations.person_idle);

      animation_speed = 0.75f;
    }

    UpdateAnimation(skin.animation, animation_speed, state.dt);
    SyncSkinnedMesh(person, entity, skin.animation);

    commit(person);
  }
}

void AnimatedEntities::UpdateAnimatedEntities(Tachyon* tachyon, State& state) {
  profile("UpdateAnimatedEntities()");

  auto& animations = state.animations;

  // Disable all animated entity meshes upfront
  for_range(0, MAX_ANIMATED_PEOPLE - 1) {
    auto& skin = state.person_skinned_meshes[i];
    auto& person = skinned_mesh(skin.mesh_index);
    auto& shirt = skinned_mesh(skin.shirt_mesh_index);

    person.disabled = true;
    shirt.disabled = true;
  }

  // Use animated meshes on-demand based on proximity to entities
  int32 usage_counter = 0;

  HandleLesserGuardAnimations(tachyon, state, usage_counter);
  HandleLowGuardAnimations(tachyon, state, usage_counter);
  HandleNPCAnimations(tachyon, state, usage_counter);
}