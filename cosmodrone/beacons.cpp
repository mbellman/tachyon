#include <map>

#include "cosmodrone/autopilot.h"
#include "cosmodrone/beacons.h"

using namespace Cosmodrone;

static inline float GetBeaconAlpha(const float progress) {
  return sqrtf(sinf(progress * t_PI));
}

void Beacons::InitBeacons(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  const static std::vector<uint16> beacon_mesh_indexes = {
    meshes.antenna_3,
    meshes.antenna_5,
    meshes.charge_pad,
    meshes.fighter_dock,
    meshes.floater_1
  };

  state.beacons.clear();

  remove_all(meshes.beacon);

  for (auto mesh_index : beacon_mesh_indexes) {
    for (auto& object : objects(mesh_index)) {
      Beacon beacon;
      beacon.beacon_1 = create(meshes.beacon);
      beacon.beacon_2 = create(meshes.beacon);
      beacon.source_object = object;

      beacon.beacon_1.rotation = object.rotation;
      beacon.beacon_2.rotation = object.rotation;
      beacon.beacon_1.scale = 1500.f;
      beacon.beacon_2.scale = 1500.f;
      beacon.beacon_1.color = tVec4f(1.f, 0.5f, 0.2f, 1.f);
      beacon.beacon_2.color = tVec4f(1.f, 0.5f, 0.2f, 1.f);

      state.beacons.push_back(beacon);
    }
  }
}

void Beacons::UpdateBeacons(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  static std::map<uint16, tVec3f> beacon_color_map = {
    { meshes.antenna_3, tVec3f(1.f, 0.6f, 0.2f) },
    { meshes.antenna_5, tVec3f(1.f, 0.6f, 0.2f) },
    { meshes.charge_pad, tVec3f(0.2f, 1.f, 0.5f) },
    { meshes.fighter_dock, tVec3f(1.f, 0.3f, 0.2f) },
    { meshes.floater_1, tVec3f(0.2f, 1.f, 0.5f) }
  };

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
    auto& beacon_color = beacon_color_map[source_object.mesh_index];

    beacon.beacon_1.position =
      docking_position -
      direction * 500.f +
      direction * 3000.f * beacon_1_progress;

    beacon.beacon_1.scale = 1500.f;

    beacon.beacon_1.color = tVec4f(
      beacon_color,
      GetBeaconAlpha(beacon_1_progress)
    );

    beacon.beacon_2.position =
      docking_position -
      direction * 500.f +
      direction * 3000.f * beacon_2_progress;

    beacon.beacon_2.scale = 1500.f;

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