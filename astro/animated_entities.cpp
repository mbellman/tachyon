#include "astro/animated_entities.h"
#include "astro/animation.h"
#include "astro/entity_behaviors/behavior.h"

using namespace astro;

struct ActiveAnimation {
  tSkeletonAnimation* animation;
  float speed;
};

// @todo update to proper animations
static ActiveAnimation GetLesserGuardActiveAnimation(Tachyon* tachyon, State& state, GameEntity& entity) {
  auto& enemy = entity.enemy_state;

  if (time_since(enemy.last_break_time) < 2.f) {
    return { &state.animations.person_idle, 1.f };
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

static void UpdateAnimation(tSkinnedMeshAnimation& animation, const float speed, const float dt) {
  Animation::AccumulateTime(animation, speed, dt);
  Animation::UpdatePose(animation);
  Animation::UpdateBoneMatrices(animation);
}

static void UpdateSkinnedMesh(tSkinnedMesh& mesh, GameEntity& entity, tSkinnedMeshAnimation& animation) {
  mesh.position = entity.visible_position;
  mesh.rotation = entity.visible_rotation;
  mesh.scale = tVec3f(1500.f);
  mesh.shadow_cascade_ceiling = 1;
  mesh.disabled = false;
  mesh.current_pose = &animation.active_pose;
}

void AnimatedEntities::UpdateAnimatedEntities(Tachyon* tachyon, State& state) {
  profile("UpdateAnimatedEntities()");

  auto& animations = state.animations;

  // Disable all animated entity meshes upfront
  for_range(0, MAX_ANIMATED_PEOPLE - 1) {
    auto& skin = state.person_skinned_meshes[i];
    auto& person = skinned_mesh(skin.mesh_index);

    person.disabled = true;
  }

  // Use animated meshes on-demand based on proximity to entities
  int32 next_index = 0;

  // @todo factor
  for_entities(state.lesser_guards) {
    auto& entity = state.lesser_guards[i];

    if (abs(state.player_position.x - entity.visible_position.x) > 15000.f) continue;
    if (abs(state.player_position.z - entity.visible_position.z) > 15000.f) continue;
    if (!IsDuringActiveTime(entity, state)) continue;

    auto& skin = state.person_skinned_meshes[next_index++];
    auto& person = skinned_mesh(skin.mesh_index);

    if (skin.animation.current_animation == nullptr) {
      skin.animation.current_animation = &animations.player_run;
    }

    auto active_animation = GetLesserGuardActiveAnimation(tachyon, state, entity);

    Animation::AwaitNextAnimation(skin.animation, active_animation.animation);

    UpdateAnimation(skin.animation, active_animation.speed, state.dt);
    UpdateSkinnedMesh(person, entity, skin.animation);

    commit(person);
  }

  // @todo factor
  for_entities(state.npcs) {
    auto& entity = state.npcs[i];

    if (abs(state.player_position.x - entity.visible_position.x) > 15000.f) continue;
    if (abs(state.player_position.z - entity.visible_position.z) > 15000.f) continue;
    if (!IsDuringActiveTime(entity, state)) continue;

    auto& skin = state.person_skinned_meshes[next_index++];
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
    UpdateSkinnedMesh(person, entity, skin.animation);

    commit(person);
  }
}