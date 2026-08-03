#include "metro/player.h"
#include "metro/constants.h"
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

void Player::Update(Tachyon* tachyon, State& state) {
  auto* active_bike = GetActiveBicycle(state);
  auto& player = objects(state.meshes.dev_mannequin)[0];

  if (active_bike != nullptr) {
    // Lock the player to the active bike
    state.previous_player_position = state.player_position;
    state.player_position = UnitVisualBikeToWorldPosition(*active_bike, tVec3f(0, 0.5f, -0.3f));

    player.position = state.player_position;
    player.rotation = active_bike->visual_rotation;
  } else {
    // Update based on free player properties
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
  }

  commit(player);
}