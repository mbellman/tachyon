#include "astro/dynamic_fauna.h"
#include "engine/tachyon_random.h"

using namespace astro;

/**
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

  while (state.butterflies.size() < 10) {
    tVec3f spawn = state.player_position + tVec3f(3000.f, 1000.f, 2000.f);

    SpawnButterfly(tachyon, state, spawn);
  }
}

static void HandleButterfly(Tachyon* tachyon, State& state, Butterfly& butterfly) {
  auto& meshes = state.meshes;
  float scene_time = get_scene_time();

  auto& left_wing = objects(meshes.butterfly_left_wing).getByIdFast(butterfly.left_wing);
  auto& right_wing = objects(meshes.butterfly_right_wing).getByIdFast(butterfly.right_wing);

  // Periodically changing state
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

  // Handling state
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
    butterfly.position += butterfly.direction * 1000.f * state.dt;
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
      abs(state.player_position.x - butterfly.position.x) > 15000.f ||
      abs(state.player_position.z - butterfly.position.z) > 15000.f
    ) {
      DestroyButterfly(tachyon, state, i);
    }
  }
}

void DynamicFauna::HandleBehavior(Tachyon* tachyon, State& state) {
  profile("DynamicFauna::HandleBehavior()");

  HandleButterflies(tachyon, state);
}