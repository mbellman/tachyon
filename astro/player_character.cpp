#include "astro/player_character.h"
#include "astro/entity_manager.h"

using namespace astro;

void PlayerCharacter::UpdatePlayer(Tachyon* tachyon, State& state, const float dt) {
  // Update facing direction
  {
    tVec3f desired_facing_direction = state.player_facing_direction;
    float turning_speed = 5.f;

    if (state.has_target) {
      // When we're focused on a target, face it and turn much more quickly
      auto& target = *EntityManager::FindEntity(state, state.target_entity);

      desired_facing_direction = (target.visible_position - state.player_position).xz().unit();
    }
    else if (state.player_velocity.magnitude() > 0.01f) {
      // Without a target, use our velocity vector to influence facing direction
      desired_facing_direction = state.player_velocity.unit();
    }

    // When astro turning, don't change our facing direction at all,
    // since targeted entities may jitter and jump about rapidly,
    // and we don't want the facing direction being thrown off
    if (abs(state.astro_turn_speed) > 0.05f) {
      turning_speed = 0.f;
    }

    state.player_facing_direction = tVec3f::lerp(state.player_facing_direction, desired_facing_direction, turning_speed * dt).unit();
  }

  Quaternion player_rotation = Quaternion::FromDirection(state.player_facing_direction, tVec3f(0, 1.f, 0));
  tMat4f player_rotation_matrix = player_rotation.toMatrix4f();

  // Character model
  {
    auto& player = objects(state.meshes.player)[0];

    player.position = state.player_position;

    // @temporary
    player.scale = tVec3f(600.f, 1500.f, 600.f);
    player.color = tVec3f(0, 0.2f, 1.f);
    player.material = tVec4f(0.9f, 0, 0, 0);

    // @temporary
    if (
      state.last_damage_time != 0.f &&
      time_since(state.last_damage_time) < 1.5f
    ) {
      player.color = tVec3f(1.f, 0, 0);
    }

    player.rotation = player_rotation;

    commit(player);
  }

  // Wand
  {
    auto& wand = objects(state.meshes.wand)[0];

    tVec3f offset = player_rotation_matrix * tVec3f(-1.f, 0, 0);

    wand.position = state.player_position + offset * 900.f;
    wand.scale = tVec3f(800.f);
    wand.rotation = player_rotation;
    wand.color = tVec3f(1.f, 0.6f, 0.2f);
    wand.material = tVec4f(1.f, 0, 0, 0.4f);

    if (
      state.spells.stun_start_time != 0.f &&
      time_since(state.spells.stun_start_time) < 3.f
    ) {
      float alpha = time_since(state.spells.stun_start_time) / 3.f;

      wand.position.y += sinf(alpha * t_PI) * 1200.f;
    }

    commit(wand);
  }

  // Astro light
  {
    auto& light = *get_point_light(state.player_light_id);

    tVec3f offset = player_rotation.toMatrix4f() * tVec3f(-1.f, 0, 0);

    light.position = state.player_position + offset * 1000.f;
    light.position.y -= 300.f;
    light.radius = 2500.f;
    light.color = tVec3f(0.5f, 0.3f, 0.6f);
    light.color = get_point_light(state.astrolabe_light_id)->color;
    light.power = 0.5f;
    light.glow_power = 0.f;

    // @todo factor (Astrolabe::)
    if (time_since(state.game_time_at_start_of_turn) < 2.f) {
      float alpha = time_since(state.game_time_at_start_of_turn) / 2.f;

      light.power += sinf(alpha * t_PI);
    }
  }
}