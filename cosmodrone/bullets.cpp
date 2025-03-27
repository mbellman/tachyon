#include "cosmodrone/bullets.h"

using namespace Cosmodrone;

// @todo move to engine
template<class T>
static inline void RemoveFromArray(std::vector<T>& array, uint32 index) {
  array.erase(array.begin() + index);
}

static float GetLastSpawnTime(const std::vector<Bullet>& bullets) {
  if (bullets.size() > 0) {
    return bullets.back().spawn_time;
  }

  return 0.f;
}

static void UpdateMachineGunBullets(Tachyon* tachyon, State& state, const float dt) {
  uint8 i = 0;

  while (i < state.machine_gun_bullets.size()) {
    auto& bullet = state.machine_gun_bullets[i];
    auto& object = *get_live_object(bullet.object);

    if (state.current_game_time - bullet.spawn_time > 5.f) {
      // Despawn
      object.scale = 0.f;

      RemoveFromArray(state.machine_gun_bullets, (uint32)i);

      continue;
    }

    object.position += bullet.direction * 500000.f * dt;

    i++;
  }

  for (auto& bullet : objects(state.meshes.bullet_1)) {
    commit(bullet);
  }
}

static void UpdateMissiles(Tachyon* tachyon, State& state, const float dt) {
  uint8 i = 0;

  while (i < state.missiles.size()) {
    auto& bullet = state.missiles[i];
    auto& object = *get_live_object(bullet.object);

    if (state.current_game_time - bullet.spawn_time > 5.f) {
      // Despawn
      object.scale = 0.f;

      RemoveFromArray(state.missiles, (uint32)i);

      continue;
    }

    object.position += bullet.direction * 200000.f * dt;

    i++;
  }

  for (auto& missile : objects(state.meshes.missile_1)) {
    commit(missile);
  }
}

void Bullets::InitBullets(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  remove_all(meshes.bullet_1);
  remove_all(meshes.missile_1);

  for (uint8 i = 0; i < 100; i++) {
    create(meshes.bullet_1);
    create(meshes.missile_1);
  }
}

void Bullets::FireMachineGuns(Tachyon* tachyon, State& state) {
  if (state.next_machine_gun_bullet_index == 100) {
    state.next_machine_gun_bullet_index = 0;
  }

  if (state.current_game_time - GetLastSpawnTime(state.machine_gun_bullets) < 0.1f) {
    return;
  }

  auto& object = objects(state.meshes.bullet_1)[state.next_machine_gun_bullet_index++];
  auto& object2 = objects(state.meshes.bullet_1)[state.next_machine_gun_bullet_index++];

  object.position =
    state.ship_position -
    state.ship_rotation_basis.up * 1400.f +
    state.ship_rotation_basis.sideways * 1900.f;

  object2.position =
    state.ship_position -
    state.ship_rotation_basis.up * 1400.f -
    state.ship_rotation_basis.sideways * 1900.f;

  object.scale =
  object2.scale = tVec3f(200.f);

  object.color =
  object2.color = tVec4f(1.f, 0.2f, 0.2f, 1.f);

  state.machine_gun_bullets.push_back({
    .direction = state.ship_rotation_basis.forward,
    .object = object,
    .spawn_time = state.current_game_time
  });

  state.machine_gun_bullets.push_back({
    .direction = state.ship_rotation_basis.forward,
    .object = object2,
    .spawn_time = state.current_game_time
  });
}

void Bullets::FireMissile(Tachyon* tachyon, State& state) {
  if (state.next_missile_index == 100) {
    state.next_missile_index = 0;
  }

  if (state.current_game_time - GetLastSpawnTime(state.missiles) < 0.5f) {
    return;
  }

  auto& object = objects(state.meshes.missile_1)[state.next_missile_index++];

  object.position = state.ship_position;
  object.scale = tVec3f(500.f);
  object.color = tVec4f(0, 0, 1.f, 1.f);

  state.missiles.push_back({
    .direction = state.ship_rotation_basis.forward,
    .object = object,
    .spawn_time = state.current_game_time
  });
}

void Bullets::UpdateBullets(Tachyon* tachyon, State& state, const float dt) {
  UpdateMachineGunBullets(tachyon, state, dt);
  UpdateMissiles(tachyon, state, dt);
}