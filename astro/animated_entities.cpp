#include "astro/animated_entities.h"
#include "astro/entity_behaviors/behavior.h"

using namespace astro;

void AnimatedEntities::UpdateAnimatedEntities(Tachyon* tachyon, State& state) {
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

    person.position = entity.visible_position;
    person.rotation = entity.visible_rotation;
    person.scale = tVec3f(1500.f);
    person.shadow_cascade_ceiling = 1;
    person.disabled = false;

    // @TEMPORARY!!!!!!!!!!!!!!
    person.current_pose = &state.player_mesh_animation.active_pose;

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

    person.position = entity.visible_position;
    person.rotation = entity.visible_rotation;
    person.scale = tVec3f(1500.f);
    person.shadow_cascade_ceiling = 1;
    person.disabled = false;

    // @TEMPORARY!!!!!!!!!!!!!!
    person.current_pose = &state.player_mesh_animation.active_pose;

    commit(person);
  }
}