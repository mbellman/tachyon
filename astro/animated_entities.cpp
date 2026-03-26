#include "astro/animated_entities.h"
#include "astro/animation.h"
#include "astro/entity_behaviors/behavior.h"

using namespace astro;

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

    float animation_speed = 10.f;

    Animation::SetNextAnimation(skin.animation, &animations.player_run);
    Animation::AccumulateTime(skin.animation, animation_speed, state.dt);
    Animation::UpdatePose(skin.animation);
    Animation::UpdateBoneMatrices(skin.animation);

    person.position = entity.visible_position;
    person.rotation = entity.visible_rotation;
    person.scale = tVec3f(1500.f);
    person.shadow_cascade_ceiling = 1;
    person.disabled = false;
    person.current_pose = &skin.animation.active_pose;

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

    Animation::AccumulateTime(skin.animation, animation_speed, state.dt);
    Animation::UpdatePose(skin.animation);
    Animation::UpdateBoneMatrices(skin.animation);

    person.position = entity.visible_position;
    person.rotation = entity.visible_rotation;
    person.scale = tVec3f(1500.f);
    person.shadow_cascade_ceiling = 1;
    person.disabled = false;
    person.current_pose = &skin.animation.active_pose;

    commit(person);
  }
}