#include <map>

#include "cosmodrone/autopilot.h"
#include "cosmodrone/beacons.h"
#include "cosmodrone/utilities.h"

using namespace Cosmodrone;

static inline float GetBeaconAlpha(const float progress) {
  return sqrtf(sinf(progress * t_PI));
}

void Beacons::InitBeacons(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  state.beacons.clear();

  remove_all(meshes.beacon);

  for (auto mesh_index : Utilities::GetDockableMeshes(state)) {
    for (auto& object : objects(mesh_index)) {
      Beacon beacon;
      beacon.beacon_1 = create(meshes.beacon);
      beacon.beacon_2 = create(meshes.beacon);
      beacon.source_object = object;

      beacon.beacon_1.rotation = object.rotation;
      beacon.beacon_2.rotation = object.rotation;
      beacon.beacon_1.scale = 1300.f;
      beacon.beacon_2.scale = 1300.f;
      beacon.beacon_1.color = tVec4f(1.f, 0.5f, 0.2f, 1.f);
      beacon.beacon_2.color = tVec4f(1.f, 0.5f, 0.2f, 1.f);

      state.beacons.push_back(beacon);
    }
  }
}

void Beacons::UpdateBeacons(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  for (auto& beacon : state.beacons) {
    if (state.is_piloting_vehicle && beacon.source_object == state.current_piloted_vehicle.root_object) {
      // Disable beacons for actively piloted vehicles
      beacon.beacon_1.scale = 0.f;
      beacon.beacon_2.scale = 0.f;

      commit(beacon.beacon_1);
      commit(beacon.beacon_2);

      continue;
    }

    auto& source_object = *get_live_object(beacon.source_object);
    // @optimize don't call get_live_object twice
    auto docking_position = Autopilot::GetDockingPosition(tachyon, state, source_object);
    auto direction = source_object.rotation.getUpDirection();
    auto beacon_1_progress = fmodf(state.current_game_time, 2.f) * 0.5f;
    auto beacon_2_progress = fmodf(state.current_game_time + 1.f, 2.f) * 0.5f;
    auto beacon_color = Utilities::GetBeaconColor(state, source_object.mesh_index);

    beacon.beacon_1.position =
      docking_position -
      direction * 500.f +
      direction * 3000.f * beacon_1_progress;

    beacon.beacon_1.scale = 1300.f;

    beacon.beacon_1.color = tVec4f(
      beacon_color,
      GetBeaconAlpha(beacon_1_progress)
    );

    beacon.beacon_2.position =
      docking_position -
      direction * 500.f +
      direction * 3000.f * beacon_2_progress;

    beacon.beacon_2.scale = 1300.f;

    beacon.beacon_2.color = tVec4f(
      beacon_color,
      GetBeaconAlpha(beacon_2_progress)
    );

    beacon.beacon_1.rotation = source_object.rotation;
    beacon.beacon_2.rotation = source_object.rotation;

    commit(beacon.beacon_1);
    commit(beacon.beacon_2);
  }
}