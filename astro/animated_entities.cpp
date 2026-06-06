#include "astro/animated_entities.h"
#include "astro/animation.h"
#include "astro/entity_behaviors/behavior.h"
#include "astro/entity_behaviors/LesserGuard.h"
#include "astro/entity_behaviors/LowGuard.h"

using namespace astro;

struct AnimationParams {
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

static void UpdateAnimation(tAnimationRig& rig, const float speed, const float dt) {
  Animation::AccumulateTime(rig, speed, 3.f, dt);
  Animation::UpdatePose(rig, BLEND_LINEAR);
  Animation::UpdateBoneMatrices(rig);
}

static void SyncSkinnedMesh(tSkinnedMesh& mesh, GameEntity& entity, tAnimationRig& animation) {
  mesh.position = entity.visible_position;
  mesh.rotation = entity.visible_rotation;
  mesh.scale = tVec3f(1500.f);
  mesh.shadow_cascade_ceiling = 1;
  mesh.disabled = false;
  mesh.current_pose = &animation.active_pose;
}

static void AttachToHead(tObject& object, SkinnedPerson& person, const tVec3f& body_position, const Quaternion& body_rotation) {
  auto& head_bone = person.rig.active_pose.bones[0];
  tVec3f scale = tVec3f(1500.f);

  tVec3f head_offset;
  head_offset += head_bone.translation * scale;
  head_offset += head_bone.rotation.toMatrix4f() * tVec3f(0, scale.y * 0.35f, 0);

  object.position = body_position + body_rotation.toMatrix4f() * head_offset;
  object.rotation = body_rotation * head_bone.rotation;
  object.scale = scale;
}

static void AttachToRightShoulder(tObject& object, SkinnedPerson& person, const tVec3f& body_position, const Quaternion& body_rotation) {
  auto& bone = person.rig.active_pose.bones[7];
  tVec3f scale = tVec3f(1500.f);

  tVec3f offset;
  offset += bone.translation * scale;

  // @hack rotate the object back to the right-side-up position,
  // above the shoulder bone
  Quaternion orientation_flip = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), t_PI);

  object.position = body_position + body_rotation.toMatrix4f() * offset;
  object.rotation = body_rotation * bone.rotation * orientation_flip;
  object.scale = scale;
}

static void AttachToRightArm(tObject& object, SkinnedPerson& person, const tVec3f& body_position, const Quaternion& body_rotation) {
  auto& bone = person.rig.active_pose.bones[5];
  tVec3f scale = tVec3f(1500.f);

  tVec3f offset;
  offset += bone.translation * scale;

  object.position = body_position + body_rotation.toMatrix4f() * offset;
  object.rotation = body_rotation * bone.rotation;
  object.scale = scale;
}

static void AttachToLeftShoulder(tObject& object, SkinnedPerson& person, const tVec3f& body_position, const Quaternion& body_rotation) {
  auto& bone = person.rig.active_pose.bones[4];
  tVec3f scale = tVec3f(1500.f);

  tVec3f offset;
  offset += bone.translation * scale;

  object.position = body_position + body_rotation.toMatrix4f() * offset;
  object.rotation = body_rotation * bone.rotation;
  object.scale = scale;
}

static void AttachToLeftArm(tObject& object, SkinnedPerson& person, const tVec3f& body_position, const Quaternion& body_rotation) {
  auto& bone = person.rig.active_pose.bones[2];
  tVec3f scale = tVec3f(1500.f);

  tVec3f offset;
  offset += bone.translation * scale;

  object.position = body_position + body_rotation.toMatrix4f() * offset;
  object.rotation = body_rotation * bone.rotation;
  object.scale = scale;
}

static void HandleAnimatedPerson(State& state, SkinnedPerson& person, AnimationParams& params) {
  auto& animations = state.animations;

  if (person.rig.current_animation == nullptr) {
    person.rig.current_animation = &animations.player_run;
  }

  if (params.immediate) {
    Animation::StartNextAnimation(person.rig, params.animation);
  } else {
    Animation::AwaitNextAnimation(person.rig, params.animation);
  }

  UpdateAnimation(person.rig, params.speed, state.dt);
}

/**
 * -------------
 * Lesser guards
 * -------------
 */
static AnimationParams GetLesserGuardAnimationParams(Tachyon* tachyon, State& state, GameEntity& entity) {
  auto& enemy = entity.enemy_state;

  if (time_since(enemy.last_damage_time) < 1.f) {
    return { &state.animations.person_hit_front, 5.f, true };
  }
  else if (time_since(enemy.last_break_time) < 1.f) {
    // @todo break animation
    return { &state.animations.person_hit_front, 5.f, true };
  }
  else if (time_since(enemy.last_attack_start_time) < LesserGuard::ATTACK_DURATION) {
    // @temporary
    return { &state.animations.player_swing_wand, 3.f, true };
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

static void HandleAnimatedLesserGuards(Tachyon* tachyon, State& state, int32& usage_counter) {
  auto& meshes = state.meshes;
  auto& animations = state.animations;

  if (state.enemies_disabled) {
    return;
  }

  for_entities(state.lesser_guards) {
    auto& entity = state.lesser_guards[i];

    if (abs(state.player_position.x - entity.visible_position.x) > 15000.f) continue;
    if (abs(state.player_position.z - entity.visible_position.z) > 15000.f) continue;
    if (!IsDuringActiveTime(entity, state)) continue;

    auto& person = state.skinned_people[usage_counter++];
    auto animation_params = GetLesserGuardAnimationParams(tachyon, state, entity);

    HandleAnimatedPerson(state, person, animation_params);

    tVec3f body_position = entity.visible_position;
    Quaternion body_rotation = entity.visible_rotation;

    // Death animation
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

    // Shirt
    {
      auto& shirt = skinned_mesh(person.shirt_mesh_index);

      SyncSkinnedMesh(shirt, entity, person.rig);

      shirt.position = body_position;
      shirt.rotation = body_rotation;
      shirt.color = tVec4f(0.5f, 0.8f, 0.7f, 0.2f);
      shirt.material = tVec4f(0.8f, 0, 0, 0.4f);

      commit(shirt);
    }

    // Helmet
    {
      auto& helmet = use_instance(meshes.lesser_helmet);

      helmet.color = tVec3f(0.7f, 0.4f, 0.1f);
      helmet.material = tVec4f(0.5f, 0, 0, 0.2f);

      AttachToHead(helmet, person, body_position, body_rotation);

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

      AttachToLeftArm(left_vambrace, person, body_position, body_rotation);
      AttachToRightArm(right_vambrace, person, body_position, body_rotation);

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
static AnimationParams GetLowGuardAnimationParams(Tachyon* tachyon, State& state, GameEntity& entity) {
  auto& enemy = entity.enemy_state;

  if (time_since(enemy.last_damage_time) < 1.f) {
    return { &state.animations.person_hit_front, 5.f, true };
  }
  else if (time_since(enemy.last_break_time) < 1.f) {
    // @todo break animation
    return { &state.animations.person_hit_front, 5.f, true };
  }
  else if (time_since(enemy.last_attack_start_time) < LowGuard::ATTACK_DURATION) {
    // @temporary
    return { &state.animations.player_swing_wand, 4.f, true };
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

static void HandleAnimatedLowGuards(Tachyon* tachyon, State& state, int32& usage_counter) {
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

    auto& person = state.skinned_people[usage_counter++];
    auto animation_params = GetLowGuardAnimationParams(tachyon, state, entity);

    HandleAnimatedPerson(state, person, animation_params);

    tVec3f body_position = entity.visible_position;
    Quaternion body_rotation = entity.visible_rotation;

    // Mail
    {
      auto& mail = skinned_mesh(person.shirt_mesh_index);

      SyncSkinnedMesh(mail, entity, person.rig);

      mail.color = tVec3f(0.5f);
      mail.material = tVec4f(0.5f, 1.f, 0, 0.4f);

      commit(mail);
    }

    // Helmet
    {
      auto& helmet = use_instance(meshes.low_helmet);

      helmet.color = tVec3f(1.f);
      helmet.material = tVec4f(0.3f, 1.f, 0, 0.2f);

      AttachToHead(helmet, person, body_position, body_rotation);

      commit(helmet);
    }

    // Shoulder plates
    {
      auto& left_plate = use_instance(meshes.shoulder_plate);
      auto& right_plate = use_instance(meshes.shoulder_plate);

      left_plate.color = tVec3f(1.f);
      left_plate.material = tVec4f(0.3f, 1.f, 0, 0.2f);

      right_plate.color = tVec3f(1.f);
      right_plate.material = tVec4f(0.3f, 1.f, 0, 0.2f);

      AttachToLeftShoulder(left_plate, person, body_position, body_rotation);
      AttachToRightShoulder(right_plate, person, body_position, body_rotation);

      commit(left_plate);
      commit(right_plate);
    }

    // Vambraces
    {
      // @todo low_vambrace
      auto& left_vambrace = use_instance(meshes.lesser_vambrace);
      auto& right_vambrace = use_instance(meshes.lesser_vambrace);

      left_vambrace.color = tVec3f(1.f);
      left_vambrace.material = tVec4f(0.3f, 1.f, 0, 0.2f);

      right_vambrace.color = tVec3f(1.f);
      right_vambrace.material = tVec4f(0.3f, 1.f, 0, 0.2f);

      AttachToLeftArm(left_vambrace, person, body_position, body_rotation);
      AttachToRightArm(right_vambrace, person, body_position, body_rotation);

      commit(left_vambrace);
      commit(right_vambrace);
    }
  }
}

/**
 * ----
 * NPCs
 * ----
 */
static void HandleAnimatedNPCs(Tachyon* tachyon, State& state, int32& usage_counter) {
  auto& meshes = state.meshes;
  auto& animations = state.animations;

  for_entities(state.npcs) {
    auto& entity = state.npcs[i];

    if (abs(state.player_position.x - entity.visible_position.x) > 15000.f) continue;
    if (abs(state.player_position.z - entity.visible_position.z) > 15000.f) continue;
    if (!IsDuringActiveTime(entity, state)) continue;

    auto& person = state.skinned_people[usage_counter++];
    auto& body = skinned_mesh(person.body_mesh_index);

    if (person.rig.current_animation == nullptr) {
      person.rig.current_animation = &animations.person_idle;
    }

    float animation_speed;

    // @todo improve determinant for current talking npc
    if (state.current_dialogue_set == entity.unique_name) {
      Animation::SetNextAnimation(person.rig, &animations.person_talking);

      animation_speed = 1.75f;
    } else {
      Animation::SetNextAnimation(person.rig, &animations.person_idle);

      animation_speed = 0.75f;
    }

    UpdateAnimation(person.rig, animation_speed, state.dt);
    SyncSkinnedMesh(body, entity, person.rig);

    commit(body);
  }
}

void AnimatedEntities::UpdateAnimatedEntities(Tachyon* tachyon, State& state) {
  profile("UpdateAnimatedEntities()");

  auto& meshes = state.meshes;
  auto& animations = state.animations;

  // Disable all animated entity meshes first. We'll re-enable
  // them on-demand as entities come into view.
  for_range(0, MAX_ANIMATED_PEOPLE - 1) {
    auto& person = state.skinned_people[i];
    auto& body = skinned_mesh(person.body_mesh_index);
    auto& shirt = skinned_mesh(person.shirt_mesh_index);

    body.disabled = true;
    shirt.disabled = true;
  }

  reset_instances(meshes.lesser_helmet);
  reset_instances(meshes.lesser_vambrace);
  reset_instances(meshes.low_helmet);
  reset_instances(meshes.shoulder_plate);

  // Use animated meshes on-demand based on proximity to entities
  int32 usage_counter = 0;

  HandleAnimatedLesserGuards(tachyon, state, usage_counter);
  HandleAnimatedLowGuards(tachyon, state, usage_counter);
  HandleAnimatedNPCs(tachyon, state, usage_counter);
}