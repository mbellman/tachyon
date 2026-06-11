#include "astro/dynamic_fauna.h"
#include "astro/entity_manager.h"
#include "astro/collision_system.h"
#include "astro/entity_behaviors/behavior.h"
#include "astro/targeting.h"
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
static void PlayWingsFlappingSound() {
  float r = Tachyon_GetRandom();

  if (r < 0.33f) Sfx::PlaySound(SFX_BIRD_WINGS_1, 0.3f);
  else if (r < 0.66f) Sfx::PlaySound(SFX_BIRD_WINGS_2, 0.3f);
  else Sfx::PlaySound(SFX_BIRD_WINGS_3, 0.3f);
}

static void PlayTweetingSound() {
  float r = Tachyon_GetRandom();

  if (r < 0.5f) Sfx::PlaySound(SFX_TWEET_1, 0.4f);
  else Sfx::PlaySound(SFX_TWEET_2, 0.4f);
}

static void SpawnTinyBird(Tachyon* tachyon, State& state, const GameEntity& spawn_entity) {
  float scene_time = get_scene_time();

  // Randomly play a sound effect when spawning
  if (Tachyon_GetRandom() < 0.33f) {
    PlayTweetingSound();
  }

  state.last_tiny_bird_spawn_time = scene_time;
  state.tiny_bird_cooldown_time = Tachyon_GetRandom(2.f, 6.f);

  // Determine target position
  tVec3f target_position = spawn_entity.position;
  target_position.x += Tachyon_GetRandom(-4000.f, 4000.f);
  target_position.z += Tachyon_GetRandom(-4000.f, 4000.f);

  float ground_height = CollisionSystem::QueryGroundHeight(state, target_position.x, target_position.z);

  TinyBird bird;

  // Pick a starting offset from which to fly in
  tVec3f offset;

  if (spawn_entity.position.y - ground_height > 6000.f) {
    // Spawn in the air, and continue flying
    bird.state = TinyBird::FLY_FORWARD;
    target_position.y = spawn_entity.position.y;

    // Spawn pseudo-randomly on the left or right side, with a random z offset
    float x = fmodf(scene_time, 1.f) > 0.5f ? 15000.f : -15000.f;
    float z = Tachyon_GetRandom(-5000.f, 5000.f);
    offset = tVec3f(x, 0.f, z);

    // Randomize the initial values for airborne birds in case
    // they spawn in small groups, to ensure variation between
    // individual birds
    bird.last_wing_flapping_time = scene_time - 0.5f * Tachyon_GetRandom();
    bird.timer = Tachyon_GetRandom();

  } else {
    // Spawn and land on the ground
    bird.state = TinyBird::FLY_DOWN_AND_LAND;
    target_position.y = ground_height + 400.f;

    // Spawn randomly around the target position
    float x = 12000.f * sinf(scene_time);
    float z = 12000.f * cosf(scene_time);
    offset = tVec3f(x, 15000.f, z);
  }

  bird.target_position = target_position;
  bird.position = target_position + offset;

  // Rotate to target position
  tVec3f bird_to_target_direction = (target_position - bird.position).xz().unit();
  Quaternion starting_rotation = Quaternion::FromDirection(bird_to_target_direction, tVec3f(0, 1.f, 0));

  bird.rotation = starting_rotation;
  bird.last_jump_time = scene_time;

  state.tiny_birds.push_back(bird);

  if (
    bird.state == TinyBird::FLY_FORWARD &&
    // Chance of spawning another airborne tiny bird with this one
    Tachyon_GetRandom() < 0.3f
  ) {
    SpawnTinyBird(tachyon, state, spawn_entity);
  }
}

static void HandleTinyBirdSpawningBehavior(Tachyon* tachyon, State& state, const float player_speed) {
  float last_spawn_time = time_since(state.last_tiny_bird_spawn_time);

  if (last_spawn_time < state.tiny_bird_cooldown_time) return;
  if (abs(state.astro_turn_speed) != 0.f) return;

  for_entities(state.bird_spawns) {
    auto& entity = state.bird_spawns[i];

    if (state.tiny_birds.size() >= 10) break;
    if (!IsDuringActiveTime(entity, state)) continue;
    if (!IsInRangeX(entity, state, 20000.f)) continue;
    if (!IsInRangeZ(entity, state, 20000.f)) continue;
    if (Targeting::IsInCombatMode(state)) continue;

    // Coin flip for whether this entity gets to spawn any birds
    if (Tachyon_GetRandom() > 0.5f) continue;

    auto proximity = GetEntityProximity(entity, state);

    if (
      proximity.distance > 15000.f ||
      (proximity.distance < 15000.f && player_speed < 200.f)
    ) {
      SpawnTinyBird(tachyon, state, entity);

      break;
    }
  }

  // @todo land on other objects
}

// @todo refactor with SmallBird.h
static void HandleTinyBirdIdling(Tachyon* tachyon, TinyBird& bird) {
  float scene_time = get_scene_time();

  // Randomly trigger a turn action whenever the bird's "mood" ""changes""
  {
    // Cycle between a few different durations to add variance to turn timings
    const float mood_durations[] = {
      0.8f,
      2.5f,
      2.f,
      5.4f,
      3.2f
    };

    int duration_cycle = int(bird.timer);
    float mood_duration = mood_durations[duration_cycle % 5];

    if (time_since(bird.last_jump_time) > mood_duration) {
      bool jump_forward = Tachyon_GetRandom() < 0.35f;

      bird.last_jump_time = get_scene_time();

      if (jump_forward) {
        // Forward hop
        // @todo fix inversion hack
        tVec3f forward_direction = bird.rotation.getDirection().invert();

        bird.state = TinyBird::JUMP_FORWARD;
        bird.jump_start_position = bird.position;
        bird.target_position = bird.position + forward_direction * 750.f;
      } else {
        // Jump in place
        bird.target_position = bird.position;
      }
    }
  }

  // Turning behavior; do a little jump and turn smoothly to a new angle
  {
    float time_since_jump = time_since(bird.last_jump_time);

    if (bird.last_jump_time != 0.f && time_since_jump < 0.2f) {
      float alpha = time_since_jump / 0.2f;

      bird.position.y = bird.target_position.y + 250.f * sinf(alpha * t_PI);

      // Pick a "random" turn angle based on the last mood change time
      float angle = t_PI * sinf(2.123f * bird.last_jump_time);
      Quaternion new_rotation = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), angle);

      bird.rotation = Quaternion::nlerp(bird.rotation, new_rotation, alpha);
    } else {
      bird.position.y = bird.target_position.y;
    }
  }

  // Fly away after idling for too long
  {
    if (bird.timer > 20.f) {
      bird.state = TinyBird::FLY_UP;

      auto& camera = tachyon->scene.camera;

      if (abs(camera.position.x - bird.position.x) < 10000.f) {
        PlayWingsFlappingSound();
      }
    }
  }
}

static void HandleTinyBirdFlyingDown(TinyBird& bird, const tVec3f& direction, const float dt) {
  Quaternion rotation = Quaternion::FromDirection(direction, tVec3f(0, 1.f, 0));
  float distance = (bird.target_position - bird.position).magnitude();
  float distance_ratio = distance / 20000.f;
  float speed = Tachyon_Lerpf(5000.f, 13000.f, distance_ratio);

  bird.position += direction * speed * dt;
  bird.position.y = bird.target_position.y + 15000.f * distance_ratio;
  bird.rotation = Quaternion::nlerp(bird.rotation, rotation, 20.f * dt);

  if (distance < 100.f) {
    bird.position = bird.target_position;
    bird.state = TinyBird::IDLING;
    bird.did_land = true;
  }
}

static void HandleTinyBirdJumpingForward(Tachyon* tachyon, TinyBird& bird) {
  float jump_alpha = time_since(bird.last_jump_time) / 0.2f;

  if (jump_alpha >= 1.f) {
    // Finish jump
    bird.state = TinyBird::IDLING;

    jump_alpha = 1.f;
  }

  bird.position.x = Tachyon_Lerpf(bird.jump_start_position.x, bird.target_position.x, jump_alpha);
  bird.position.y = bird.target_position.y + 300.f * sinf(jump_alpha * t_PI);
  bird.position.z = Tachyon_Lerpf(bird.jump_start_position.z, bird.target_position.z, jump_alpha);
}

static void HandleTinyBirdFlyingAway(TinyBird& bird, const tVec3f& direction, const float dt) {
  Quaternion away_rotation = Quaternion::FromDirection(direction, tVec3f(0, 1.f, 0));

  bird.position += direction * 13000.f * dt;
  bird.position.y += 7000.f * dt;
  bird.rotation = Quaternion::nlerp(bird.rotation, away_rotation, 20.f * dt);
}

static void HandleTinyBirdFlyingForward(TinyBird& bird, const tVec3f& direction, const float dt) {
  Quaternion rotation = Quaternion::FromDirection(direction, tVec3f(0, 1.f, 0));

  bird.position += direction * 13000.f * dt;
  bird.position.y += 1000.f * sinf(bird.position.x * 0.00025f) * dt;
  bird.rotation = Quaternion::nlerp(bird.rotation, rotation, 20.f * dt);

  if (bird.flapping_wings) {
    // Pitch up based on wing speed
    float pitch_delta = -0.9f * (bird.wing_speed / 60.f) * dt;

    bird.rotation = bird.rotation * Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), pitch_delta);
  } else {
    // Pitch down
    float pitch_delta = 0.6f * dt;

    bird.rotation = bird.rotation * Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), pitch_delta);
  }

  // Turn back and forth, and roll accordingly
  float turn_delta = 0.35f * sinf(bird.timer * 3.f) * dt;
  float roll_delta = -40.f * turn_delta;

  bird.rotation *= Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), turn_delta);
  bird.rotation = bird.rotation * Quaternion::fromAxisAngle(tVec3f(0, 0, 1.f), roll_delta);
}

static void HandleTinyBird(Tachyon* tachyon, State& state, TinyBird& bird, const float player_speed) {
  auto& meshes = state.meshes;
  const tVec3f bird_color = tVec3f(0.3f, 0.2f, 0.1f);
  const tVec3f body_color = tVec3f(0.3f);
  const tVec3f wing_color = tVec3f(0.2f, 0.14f, 0.07f);

  bird.timer += state.dt;

  tVec3f player_to_bird = bird.position - state.player_position;
  float player_distance = player_to_bird.magnitude();

  // Change state based on player proximity
  {
    if (
      bird.state != TinyBird::FLY_FORWARD &&
      player_distance < 5000.f &&
      player_speed > 200.f
    ) {
      bird.state = TinyBird::FLY_UP;

      PlayWingsFlappingSound();

      if (Tachyon_GetRandom() < 0.33f) {
        PlayTweetingSound();
      }
    }
  }

  // Per-state behavior
  {
    if (bird.state == TinyBird::IDLING) {
      HandleTinyBirdIdling(tachyon, bird);
    }

    else if (bird.state == TinyBird::JUMP_FORWARD) {
      HandleTinyBirdJumpingForward(tachyon, bird);
    }

    else if (bird.state == TinyBird::FLY_DOWN_AND_LAND) {
      tVec3f fly_direction = (bird.target_position - bird.position).xz().unit();

      HandleTinyBirdFlyingDown(bird, fly_direction, state.dt);
    }

    else if (bird.state == TinyBird::FLY_UP) {
      tVec3f fly_direction;

      if (bird.did_land) {
        // Fly away from the player
        fly_direction = player_to_bird.xz().unit();
      } else {
        // Continue flying along the current direction
        fly_direction = bird.rotation.getDirection().invert();
      }

      HandleTinyBirdFlyingAway(bird, fly_direction, state.dt);
    }

    else if (bird.state == TinyBird::FLY_FORWARD) {
      tVec3f fly_direction = bird.rotation.getDirection().invert();

      HandleTinyBirdFlyingForward(bird, fly_direction, state.dt);
    }
  }

  // Body
  {
    auto& body = use_instance(meshes.tiny_bird_body);

    body.position = bird.position;
    body.rotation = bird.rotation;
    body.scale = tVec3f(750.f);
    body.color = body_color;
    body.material = tVec4f(0.8f, 0, 0, 0.6f);

    commit(body);
  }

  // Head
  {
    auto& head = use_instance(meshes.tiny_bird_head);

    head.position = bird.position;
    head.rotation = bird.rotation;
    head.scale = tVec3f(750.f);
    head.color = bird_color;
    head.material = tVec4f(0.8f, 0, 0, 0.6f);

    if (bird.state == TinyBird::IDLING) {
      // Head turning
      // @todo factor
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
    } else {
      head.rotation = bird.rotation;
    }

    commit(head);
  }

  // Wings (not flying)
  if (
    bird.state != TinyBird::FLY_DOWN_AND_LAND &&
    bird.state != TinyBird::FLY_UP &&
    bird.state != TinyBird::FLY_FORWARD
  ) {
    auto& wings = use_instance(meshes.tiny_bird_wings);

    wings.position = bird.position;
    wings.rotation = bird.rotation;
    wings.scale = tVec3f(750.f);
    wings.color = wing_color;
    wings.material = tVec4f(0.8f, 0, 0, 0.6f);

    commit(wings);

  // Wings (flying)
  } else {
    if (bird.state == TinyBird::FLY_FORWARD) {
      // When flying forward, have the bird alternate between
      // flapping its wings and keeping them steady
      // @todo factor all this

      if (bird.flapping_wings) {
        if (bird.wing_value < t_TAU) {
          // Approach top wing speed
          bird.wing_speed = Tachyon_Lerpf(bird.wing_speed, 80.f, 3.f * state.dt);
        } else {
          // Start slowing wings down after a few cycles
          bird.wing_speed = Tachyon_Lerpf(bird.wing_speed, 1.f, 3.f * state.dt);
        }

        // Accumulate wing value and determine angle
        bird.wing_value += bird.wing_speed * state.dt;

        float target_angle = 0.25f + 0.75f * sinf(bird.wing_value);
        float blend = bird.wing_speed * state.dt;

        clamp_to_1(blend);

        bird.wing_angle = Tachyon_Lerpf(bird.wing_angle, target_angle, blend);

        // Stop flapping wings every few cycles
        if (bird.wing_value > t_TAU * 2.f + t_PI) {
          bird.last_wing_flapping_time = get_scene_time();
          bird.steady_flight_duration = Tachyon_GetRandom(0.3f, 0.55f);
          bird.flapping_wings = false;
        }
      } else {
        // Reset wing speed/value, converge wing angle to steady flight
        bird.wing_speed = 0.f;
        bird.wing_value = 0.f;
        bird.wing_angle = Tachyon_Lerpf(bird.wing_angle, -1.f, 1.5f * state.dt);

        // Start flapping wings after a period of steady flight
        if (time_since(bird.last_wing_flapping_time) > bird.steady_flight_duration) {
          bird.flapping_wings = true;
        }
      }
    } else {
      // Use a constant flap rate when flying down or up
      bird.wing_angle = sinf(45.f * bird.timer);
    }

    // Left
    {
      auto& left_wing = use_instance(meshes.tiny_bird_left_wing);

      left_wing.position = bird.position;
      left_wing.rotation = bird.rotation * Quaternion::fromAxisAngle(tVec3f(0, 0, 1.f), -bird.wing_angle);
      left_wing.scale = tVec3f(900.f);
      left_wing.color = wing_color;
      left_wing.material = tVec4f(0.8f, 0, 0, 0.6f);

      commit(left_wing);
    }

    // Right
    {
      auto& right_wing = use_instance(meshes.tiny_bird_right_wing);

      right_wing.position = bird.position;
      right_wing.rotation = bird.rotation * Quaternion::fromAxisAngle(tVec3f(0, 0, 1.f), bird.wing_angle);
      right_wing.scale = tVec3f(900.f);
      right_wing.color = wing_color;
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
      abs(bird.position.z - state.player_position.z) > 25000.f ||
      state.astro_turn_speed != 0.f
    ) {
      birds.erase(birds.begin() + i);
    }
  }
}

/**
 * ----------
 * Ducks
 * ----------
 */
static tVec3f GetNewDuckTargetPosition(State& state, Duck& duck) {
  float offset_x = Tachyon_GetRandom(-2000.f, 2000.f);
  float offset_z = Tachyon_GetRandom(-2000.f, 2000.f);

  GameEntity& spawn_entity = *EntityManager::FindEntity(state, duck.spawn_entity_record);

  tVec3f new_target_position = spawn_entity.position;
  // @todo use local water plane position
  new_target_position.y = -3000.f;
  new_target_position.x += offset_x;
  new_target_position.z += offset_z;

  return new_target_position;
}

static float GetRotationSpeed(const float time_since_target) {
  float speed = time_since_target / 1.f;
  if (speed < 0.f) speed = 0.f;
  if (speed > 1.f) speed = 1.f;
  speed *= 0.5f;

  return speed;
}

static void HandleDuck(Tachyon* tachyon, State& state, Duck& duck) {
  auto& meshes = state.meshes;

  GameEntity& spawn_entity = *EntityManager::FindEntity(state, duck.spawn_entity_record);

  // @temporary
  if (!IsDuringActiveTime(spawn_entity, state)) {
    return;
  }

  // Update rotation/position
  {
    tVec3f target_direction = (duck.target_position - duck.position).unit();
    Quaternion target_rotation = Quaternion::FromDirection(target_direction, tVec3f(0, 1.f, 0));
    float body_rotation_speed = GetRotationSpeed(time_since(duck.last_target_time) - 0.75f);
    float head_rotation_speed = GetRotationSpeed(time_since(duck.last_target_time));

    if (time_since(duck.last_target_time) > 1.f) {
      duck.rotation = Quaternion::slerp(duck.rotation, target_rotation, body_rotation_speed * state.dt);
    }

    duck.head_rotation = Quaternion::slerp(duck.head_rotation, target_rotation, head_rotation_speed * state.dt);

    // Slow down to 200 so we don't get stuck circling the target position
    duck.current_speed = Tachyon_Lerpf(duck.current_speed, 200.f, 0.25f * state.dt);
    duck.position += duck.rotation.getDirection().invert() * duck.current_speed * state.dt;

    if (tVec3f::distance(duck.position, duck.target_position) < 500.f) {
      duck.target_position = GetNewDuckTargetPosition(state, duck);
      duck.last_target_time = get_scene_time();
      duck.current_speed = Tachyon_GetRandom(250.f, 750.f);
    }
  }

  // Body
  {
    auto& body = use_instance(meshes.duck_body);

    body.position = duck.position;
    body.rotation = duck.rotation;
    body.material = tVec4f(0.7f, 0, 0, 0.2f);
    body.scale = tVec3f(500.f);

    commit(body);
  }

  // Neck
  {
    auto& neck = use_instance(meshes.duck_neck);

    neck.position = duck.position;
    neck.rotation = duck.rotation;
    neck.color = tVec3f(0.4f, 0.2f, 0.1f);
    neck.material = tVec4f(0.8f, 0, 0, 0.2f);
    neck.scale = tVec3f(500.f);

    commit(neck);
  }

  // Wings
  {
    auto& wings = use_instance(meshes.duck_wings);

    wings.position = duck.position;
    wings.rotation = duck.rotation;
    wings.color = tVec3f(0.7f, 0.5f, 0.4f);
    wings.material = tVec4f(0.6f, 0.5f, 0, 0.4f);
    wings.scale = tVec3f(500.f);

    commit(wings);
  }

  // Head
  {
    auto& head = use_instance(meshes.duck_head);

    head.position = duck.position + duck.rotation.toMatrix4f() * tVec3f(0, 0, 650.f);
    head.rotation = duck.head_rotation;
    head.color = tVec3f(0.1f, 0.5f, 0.3f);
    head.material = tVec4f(0.5f, 1.f, 0, 0.2f);
    head.scale = tVec3f(500.f);

    commit(head);
  }

  // Beak
  {
    auto& beak = use_instance(meshes.duck_beak);

    beak.position = duck.position + duck.rotation.toMatrix4f() * tVec3f(0, 0, 650.f);
    beak.rotation = duck.head_rotation;
    beak.color = tVec3f(1.f, 0.7f, 0.3f);
    beak.scale = tVec3f(500.f);

    commit(beak);
  }
}

static void HandleDucks(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  reset_instances(meshes.duck_body);
  reset_instances(meshes.duck_neck);
  reset_instances(meshes.duck_wings);
  reset_instances(meshes.duck_head);
  reset_instances(meshes.duck_beak);

  // @todo spawn ducks in and out as we move around the map
  if (state.ducks.size() != state.duck_spawns.size()) {
    state.ducks.clear();

    for (auto& entity : state.duck_spawns) {
      Duck duck;
      duck.spawn_entity_record = GetRecord(entity);

      duck.position = entity.position;
      // @todo use local water plane position
      duck.position.y = -3000.f;
      duck.target_position = GetNewDuckTargetPosition(state, duck);
      duck.rotation = Quaternion(1.f, 0, 0, 0);
      duck.head_rotation = Quaternion(1.f, 0, 0, 0);

      state.ducks.push_back(duck);
    }
  }

  for (auto& duck : state.ducks) {
    HandleDuck(tachyon, state, duck);
  }
}

/**
 * ----------
 * Swans
 * ----------
 */
static tVec3f GetNewSwanTargetPosition(State& state, Swan& swan) {
  float offset_x = Tachyon_GetRandom(-4000.f, 4000.f);
  float offset_z = Tachyon_GetRandom(-4000.f, 4000.f);

  // @todo account for spawn entities being deleted + delete associated swans
  GameEntity& spawn_entity = *EntityManager::FindEntity(state, swan.spawn_entity_record);

  tVec3f new_target_position = spawn_entity.position;
  // @todo use local water plane position
  new_target_position.y = -3000.f;
  new_target_position.x += offset_x;
  new_target_position.z += offset_z;

  return new_target_position;
}


static void HandleSwan(Tachyon* tachyon, State& state, Swan& swan) {
  auto& meshes = state.meshes;

  // Update rotation/position
  {
    tVec3f target_direction = (swan.target_position - swan.position).unit();
    Quaternion target_rotation = Quaternion::FromDirection(target_direction, tVec3f(0, 1.f, 0));
    float body_rotation_speed = GetRotationSpeed(time_since(swan.last_target_time) - 0.75f);
    float head_rotation_speed = GetRotationSpeed(time_since(swan.last_target_time));

    if (time_since(swan.last_target_time) > 1.f) {
      swan.rotation = Quaternion::slerp(swan.rotation, target_rotation, body_rotation_speed * state.dt);
    }

    swan.head_rotation = Quaternion::slerp(swan.head_rotation, target_rotation, head_rotation_speed * state.dt);
    swan.position += swan.rotation.getDirection().invert() * 300.f * state.dt;

    if (tVec3f::distance(swan.position, swan.target_position) < 500.f) {
      swan.target_position = GetNewSwanTargetPosition(state, swan);
      swan.last_target_time = get_scene_time();
    }
  }

  // Body
  {
    auto& body = use_instance(meshes.swan_body);

    body.position = swan.position;
    body.rotation = swan.rotation;
    body.scale = tVec3f(500.f);
    body.color = tVec4f(tVec3f(1.f), 0.3f);
    body.material = tVec4f(1.f, 0, 0, 0.4f);

    commit(body);
  }

  // Beak
  {
    auto& beak = use_instance(meshes.swan_beak);

    beak.position = swan.position;
    beak.rotation = swan.rotation;
    beak.scale = tVec3f(500.f);
    beak.color = tVec3f(1.f, 0.7f, 0.3f);
    beak.material = tVec4f(1.f, 0, 0, 0.4f);

    commit(beak);
  }

  // Beak skin
  {
    auto& beak_skin = use_instance(meshes.swan_beak_skin);

    beak_skin.position = swan.position;
    beak_skin.rotation = swan.rotation;
    beak_skin.scale = tVec3f(500.f);
    beak_skin.color = tVec3f(0.f);
    beak_skin.material = tVec4f(1.f, 0, 0, 0);

    commit(beak_skin);
  }
}

static void HandleSwans(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  reset_instances(meshes.swan_body);
  reset_instances(meshes.swan_beak);
  reset_instances(meshes.swan_beak_skin);

  // @todo spawn swans in and out as we move around the map
  if (state.swans.size() != state.swan_spawns.size()) {
    state.swans.clear();

    for (auto& entity : state.swan_spawns) {
      Swan swan;
      swan.spawn_entity_record = GetRecord(entity);

      swan.position = entity.position;
      // @todo use local water plane position
      swan.position.y = -3000.f;
      swan.target_position = GetNewSwanTargetPosition(state, swan);
      swan.rotation = Quaternion(1.f, 0, 0, 0);
      swan.head_rotation = Quaternion(1.f, 0, 0, 0);

      state.swans.push_back(swan);
    }
  }

  for (auto& swan : state.swans) {
    HandleSwan(tachyon, state, swan);
  }
}

void DynamicFauna::HandleBehavior(Tachyon* tachyon, State& state) {
  profile("DynamicFauna::HandleBehavior()");

  HandleButterflies(tachyon, state);
  HandleTinyBirds(tachyon, state);
  HandleDucks(tachyon, state);
  HandleSwans(tachyon, state);
}