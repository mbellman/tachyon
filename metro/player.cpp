#include "metro/player.h"
#include "metro/utilities.h"

using namespace metro;

static void ShowCharacterDebugVisuals(Tachyon* tachyon, State& state) {
  auto& camera = tachyon->scene.camera;
  auto& player = objects(state.meshes.dev_mannequin)[0];
  tVec3f facing_direction = player.rotation.getDirection().invert();

  tVec3f ground_forward = camera.orientation.getDirection().xz().unit();
  tVec3f ground_left = tVec3f::cross(Y_UP, ground_forward);

  tVec3f velocity_position = state.player_position + tVec3f(0, 500.f, 0);
  tVec3f velocity_vector = state.player_velocity * 0.5f;

  tVec3f facing_position = state.player_position + tVec3f(0, 1000.f, 0);
  tVec3f facing_vector = facing_direction * 2000.f;

  Debug::ShowDebugVector(tachyon, state.player_position, ground_forward * 2000.f, tVec3f(1.f, 0, 0));
  Debug::ShowDebugVector(tachyon, state.player_position, ground_forward.invert() * 2000.f, tVec3f(1.f, 0, 0));

  Debug::ShowDebugVector(tachyon, state.player_position, ground_left * 2000.f, tVec3f(1.f, 0, 0));
  Debug::ShowDebugVector(tachyon, state.player_position, ground_left.invert() * 2000.f, tVec3f(1.f, 0, 0));

  Debug::ShowDebugVector(tachyon, velocity_position, velocity_vector, tVec3f(0, 0, 1.f));
  Debug::ShowDebugVector(tachyon, facing_position, facing_vector, tVec3f(0, 1.f, 0));
}

static void UpdatePlayerOnBike(Tachyon* tachyon, State& state, const Bicycle& bike) {
  auto& player = objects(state.meshes.dev_mannequin)[0];

  state.previous_player_position = state.player_position;
  state.player_position = UnitVisualBikeToWorldPosition(bike, tVec3f(0, 0.5f, -0.3f));

  player.position = state.player_position;
  player.rotation = bike.visual_rotation;

  commit(player);
}

static void UpdatePlayerOnFoot(Tachyon* tachyon, State& state) {
  auto& player = objects(state.meshes.dev_mannequin)[0];
  bool has_collision = false;

  // Collisions
  {
    for_static_entity_containers() {
      for_entities() {
        // @todo skip collision checks against out-of-range entities

        tVec3f ray_start = state.player_position;
        // @todo use a constant based on player scale + padding
        tVec3f down_ray = tVec3f(0, -2500.f, 0);

        for (auto& plane : entity.collision_planes) {
          auto test = Collision::TestRayHit(ray_start, down_ray, plane);

          if (test.has_collision) {
            state.player_position.y = test.collision_point.y + 2000.f;

            has_collision = true;
          }
        }
      }
    }
  }

  if (has_collision) {
    state.player_velocity.y = 0.f;
  } else {
    state.player_velocity.y -= 50000.f * state.dt;
  }

  player.position = state.player_position;

  if (state.recorded_player_speed > 0.f) {
    player.rotation = Quaternion::nlerp(
      player.rotation,
      Quaternion::FromDirection(state.player_velocity.unit(), Y_UP),
      5.f * state.dt
    );
  }

  if (tachyon->show_timing_profile) {
    ShowCharacterDebugVisuals(tachyon, state);
  }

  commit(player);
}

void Player::Update(Tachyon* tachyon, State& state) {
  auto* active_bike = GetActiveBicycle(state);

  if (active_bike != nullptr) {
    UpdatePlayerOnBike(tachyon, state, *active_bike);
  } else {
    UpdatePlayerOnFoot(tachyon, state);
  }
}