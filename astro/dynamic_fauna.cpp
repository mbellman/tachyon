#include "astro/dynamic_fauna.h"
#include "astro/collision_system.h"
#include "astro/entity_behaviors/behavior.h"
#include "engine/tachyon_random.h"

using namespace astro;

/**
 * -----------
 * Butterflies
 * -----------
 */
static void SpawnButterfly(Tachyon* tachyon, State& state, const tVec3f& position) {
  auto& meshes = state.meshes;

  Butterfly butterfly;
  butterfly.position = position;
  butterfly.direction = tVec3f(0, 0, -1.f);
  butterfly.left_wing = create(meshes.butterfly_left_wing).object_id;
  butterfly.right_wing = create(meshes.butterfly_right_wing).object_id;
  butterfly.state = Butterfly::TURNING_LEFT;

  state.butterflies.push_back(butterfly);
}

static void DestroyButterfly(Tachyon* tachyon, State& state, int32 index) {
  auto& meshes = state.meshes;
  auto& butterfly = state.butterflies[index];

  remove_object(meshes.butterfly_left_wing, butterfly.left_wing);
  remove_object(meshes.butterfly_right_wing, butterfly.right_wing);

  state.butterflies.erase(state.butterflies.begin() + index);
}

static void HandleButterflySpawningBehavior(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  while (state.butterflies.size() < 5) {
    tVec3f offset;
    offset.x = Tachyon_GetRandom() > 0.5f ? -15000.f : 15000.f;
    offset.y = 1000.f;
    offset.z = Tachyon_GetRandom() > 0.5f ? -3000.f : 3000.f;

    tVec3f spawn = state.player_position + offset;

    SpawnButterfly(tachyon, state, spawn);
  }
}

static void HandleButterfly(Tachyon* tachyon, State& state, Butterfly& butterfly) {
  auto& meshes = state.meshes;
  float scene_time = get_scene_time();

  auto& left_wing = objects(meshes.butterfly_left_wing).getByIdFast(butterfly.left_wing);
  auto& right_wing = objects(meshes.butterfly_right_wing).getByIdFast(butterfly.right_wing);

  // Periodically changing direction
  {
    if (time_since(butterfly.last_state_change_time) > 1.f) {
      float random = Tachyon_GetRandom();

      if (random < 0.33f) {
        butterfly.state = Butterfly::State::TURNING_LEFT;
      }
      else if (random < 0.66f) {
        butterfly.state = Butterfly::State::TURNING_RIGHT;
      }
      else {
        butterfly.state = Butterfly::State::FLYING_STRAIGHT;
      }

      butterfly.last_state_change_time = scene_time;
    }
  }

  // Avoiding collision with the player + taller objects
  {
    // @todo
  }

  // Handling direction
  {
    switch (butterfly.state) {
      case Butterfly::State::TURNING_LEFT: {
        tVec3f left = tVec3f::cross(butterfly.direction, tVec3f(0, 1.f, 0)).invert();
        tVec3f new_direction = tVec3f::lerp(butterfly.direction, left, state.dt).unit();

        butterfly.direction = new_direction;

        break;
      }

      case Butterfly::State::TURNING_RIGHT: {
        tVec3f right = tVec3f::cross(butterfly.direction, tVec3f(0, 1.f, 0));
        tVec3f new_direction = tVec3f::lerp(butterfly.direction, right, state.dt).unit();

        butterfly.direction = new_direction;

        break;
      }

      case Butterfly::State::FLYING_STRAIGHT:
        break;
    }
  }

  // Flapping wings
  {
    float angle = 0.4f * sinf(25.f * scene_time);

    left_wing.rotation = Quaternion::fromAxisAngle(tVec3f(0, 0, 1.f), angle);
    right_wing.rotation = Quaternion::fromAxisAngle(tVec3f(0, 0, 1.f), -angle);
  }

  // Updating rotation
  {
    Quaternion facing_rotation = Quaternion::FromDirection(butterfly.direction, tVec3f(0, 1.f, 0));

    left_wing.rotation *= facing_rotation;
    right_wing.rotation *= facing_rotation;
  }

  // Updating position
  {
    float speed = 1000.f + state.astro_turn_speed * 100000.f;

    butterfly.position += butterfly.direction * speed * state.dt;

    // Oscillation
    butterfly.position += 400.f * sinf(2.f * scene_time) * state.dt;
  }

  // Updating properties
  {
    left_wing.position = butterfly.position;
    left_wing.color = tVec4f(1.f, 0.7f, 0.7f, 0.4f);
    left_wing.scale = tVec3f(150.f);
    left_wing.material = tVec4f(0.8f, 0, 0, 1.f);

    right_wing.position = butterfly.position;
    right_wing.color = tVec4f(1.f, 0.7f, 0.7f, 0.4f);
    right_wing.scale = tVec3f(150.f);
    right_wing.material = tVec4f(0.8f, 0, 0, 1.f);
  }

  commit(left_wing);
  commit(right_wing);
}

static void HandleButterflies(Tachyon* tachyon, State& state) {
  HandleButterflySpawningBehavior(tachyon, state);

  // Behavior
  for (auto& butterfly : state.butterflies) {
    HandleButterfly(tachyon, state, butterfly);
  }

  // Destruction
  for (int32 i = state.butterflies.size() - 1; i >= 0; i--) {
    auto& butterfly = state.butterflies[i];

    if (
      abs(state.player_position.x - butterfly.position.x) > 20000.f ||
      abs(state.player_position.z - butterfly.position.z) > 20000.f
    ) {
      DestroyButterfly(tachyon, state, i);
    }
  }
}

/**
 * ----------
 * Tiny birds
 * ----------
 */
static void SpawnTinyBird(Tachyon* tachyon, State& state, const GameEntity& spawn_entity) {
  state.last_tiny_bird_spawn_time = get_scene_time();

  TinyBird bird;
  bird.position = spawn_entity.position;
  bird.position.x += Tachyon_GetRandom(-4000.f, 4000.f);
  bird.position.z += Tachyon_GetRandom(-4000.f, 4000.f);
  bird.position.y = CollisionSystem::QueryGroundHeight(state, bird.position.x, bird.position.z);
  bird.position.y += 400.f;

  bird.rotation = Quaternion(1.f, 0, 0, 0);

  bird.state = TinyBird::IDLING;

  state.tiny_birds.push_back(bird);
}

static void HandleTinyBirdSpawningBehavior(Tachyon* tachyon, State& state, const float player_speed) {
  float last_spawn_time = time_since(state.last_tiny_bird_spawn_time);

  if (last_spawn_time < 5.3f) return;

  for_entities(state.bird_spawns) {
    auto& entity = state.bird_spawns[i];

    if (!IsDuringActiveTime(entity, state)) continue;
    if (!IsInRangeX(entity, state, 20000.f)) continue;
    if (!IsInRangeZ(entity, state, 20000.f)) continue;
    if (state.tiny_birds.size() >= 10) break;

    // Coin flip for whether this entity gets to spawn any birds
    if (Tachyon_GetRandom() > 0.5f) continue;

    auto proximity = GetEntityProximity(entity, state);

    if (
      proximity.distance > 10000.f ||
      (proximity.distance < 10000.f && player_speed < 200.f)
    ) {
      SpawnTinyBird(tachyon, state, entity);
    }
  }
}

static void HandleTinyBird(Tachyon* tachyon, State& state, TinyBird& bird, const float player_speed) {
  auto& meshes = state.meshes;
  const tVec3f bird_color = tVec3f(0.3f, 0.2f, 0.1f);

  bird.timer += state.dt;

  tVec3f player_to_bird = bird.position - state.player_position;
  float player_distance = player_to_bird.magnitude();

  // Change state based on player proximity
  {
    if (player_distance < 5000.f && player_speed > 200.f) {
      bird.state = TinyBird::FLY_UP;

      // @todo refactor
      {
        float r = Tachyon_GetRandom();

        if (r < 0.33f) Sfx::PlaySound(SFX_BIRD_WINGS_1, 0.3f);
        else if (r < 0.66f) Sfx::PlaySound(SFX_BIRD_WINGS_2, 0.3f);
        else Sfx::PlaySound(SFX_BIRD_WINGS_3, 0.3f);
      }
    }
  }

  // Per-state behavior
  {
    if (bird.state == TinyBird::FLY_UP) {
      // @todo factor
      tVec3f away_direction = player_to_bird.xz().unit();

      bird.position += away_direction * 15000.f * state.dt;
      bird.position.y += 5000.f * state.dt;

      Quaternion away_rotation = Quaternion::FromDirection(away_direction, tVec3f(0, 1.f, 0));

      bird.rotation = Quaternion::nlerp(bird.rotation, away_rotation, 20.f * state.dt);
    }
  }

  // Body
  {
    auto& body = use_instance(meshes.tiny_bird_body);

    body.position = bird.position;
    body.rotation = bird.rotation;
    body.scale = tVec3f(750.f);
    body.color = bird_color;

    commit(body);
  }

  // Head
  {
    auto& head = use_instance(meshes.tiny_bird_head);

    head.position = bird.position;
    head.rotation = bird.rotation;
    head.scale = tVec3f(750.f);
    head.color = bird_color;

    // Head turning
    // @todo factor
    {
      static const Quaternion head_rotations[] = {
        Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), 0),
        Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), 0.3f),
        Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), -0.3f),
        Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), 0.5f) * Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), -0.3f),
        Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), -0.5f) * Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), -0.3f)
      };

      const float swivel_duration = 0.08f;

      int previous_rotation_index = int(bird.timer) % 5 - 1;
      if (previous_rotation_index < 0) previous_rotation_index = 4;

      int rotation_index = int(bird.timer) % 5;

      Quaternion old_rotation = head_rotations[previous_rotation_index];
      Quaternion new_rotation = head_rotations[rotation_index];

      float alpha = std::min(fmodf(bird.timer, 1.f), swivel_duration);
      alpha *= 1.f / swivel_duration;
      alpha = Tachyon_EaseOutQuad(alpha);

      head.rotation = bird.rotation * Quaternion::nlerp(old_rotation, new_rotation, alpha);
    }

    commit(head);
  }

  // Wings (not flying)
  if (bird.state != TinyBird::FLY_DOWN && bird.state != TinyBird::FLY_UP) {
    auto& wings = use_instance(meshes.tiny_bird_wings);

    wings.position = bird.position;
    wings.rotation = bird.rotation;
    wings.scale = tVec3f(750.f);
    wings.color = bird_color;

    commit(wings);

  // Wings (flying)
  } else {
    float flap_rate = 45.f;
    float angle = sinf(flap_rate * get_scene_time());

    // Left
    {
      auto& left_wing = use_instance(meshes.tiny_bird_left_wing);

      left_wing.position = bird.position;
      left_wing.rotation = bird.rotation * Quaternion::fromAxisAngle(tVec3f(0, 0, 1.f), -angle);
      left_wing.scale = tVec3f(750.f);
      left_wing.color = bird_color;
      left_wing.material = tVec4f(0.8f, 0, 0, 0.6f);

      commit(left_wing);
    }

    // Right
    {
      auto& right_wing = use_instance(meshes.tiny_bird_right_wing);

      right_wing.position = bird.position;
      right_wing.rotation = bird.rotation * Quaternion::fromAxisAngle(tVec3f(0, 0, 1.f), angle);
      right_wing.scale = tVec3f(750.f);
      right_wing.color = bird_color;
      right_wing.material = tVec4f(0.8f, 0, 0, 0.6f);

      commit(right_wing);
    }
  }
}

static void HandleTinyBirds(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;
  float player_speed = state.player_velocity.magnitude();

  HandleTinyBirdSpawningBehavior(tachyon, state, player_speed);

  reset_instances(meshes.tiny_bird_head);
  reset_instances(meshes.tiny_bird_body);
  reset_instances(meshes.tiny_bird_wings);
  reset_instances(meshes.tiny_bird_left_wing);
  reset_instances(meshes.tiny_bird_right_wing);

  auto& birds = state.tiny_birds;

  if (birds.size() == 0) return;

  // Behavior
  for (auto& bird : birds) {
    HandleTinyBird(tachyon, state, bird, player_speed);
  }

  // Despawn tiny birds out of range
  for (int i = birds.size() - 1; i >= 0; i--) {
    auto& bird = birds[i];

    if (
      abs(bird.position.x - state.player_position.x) > 25000.f ||
      abs(bird.position.z - state.player_position.z) > 25000.f
    ) {
      birds.erase(birds.begin() + i);
    }
  }
}

void DynamicFauna::HandleBehavior(Tachyon* tachyon, State& state) {
  profile("DynamicFauna::HandleBehavior()");

  HandleButterflies(tachyon, state);
  HandleTinyBirds(tachyon, state);
}