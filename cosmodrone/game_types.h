#pragma once

#define USE_PROCEDURAL_GENERATION 1

#include "engine/tachyon.h"
#include "cosmodrone/mesh_ids.h"

namespace Cosmodrone {
  struct OrthonormalBasis {
    tVec3f forward = tVec3f(0, 0, -1.f);
    tVec3f up = tVec3f(0, 1.f, 0);
    tVec3f sideways = tVec3f(1.f, 0, 0);
  };

  enum FlightMode {
    MANUAL_CONTROL,
    AUTO_PROGRADE,
    AUTO_RETROGRADE,
    AUTO_DOCK
  };

  enum AutoDockStage {
    APPROACH_DECELERATION,
    APPROACH_ALIGNMENT,
    APPROACH,
    DOCKING,
    DOCKED
  };

  enum FlightSystem {
    DRONE,
    FIGHTER
  };

  struct TargetTracker {
    tObject object;
    float activated_time = 0.f;
    float selected_time = 0.f;
    float deselected_time = 0.f;
    float deactivated_time = 0.f;

    int32 screen_x = 0;
    int32 screen_y = 0;
    float center_distance = 0.f;
    float object_distance = 0.f;
  };

  struct TargetStats {
    uint32 distance_in_meters;
    tVec3f unit_direction;
    float relative_velocity;
  };

  struct FlightPathNode {
    tVec3f position;
    float distance;
    tVec3f spawn_position;
    float spawn_distance;
  };

  struct BackgroundVehicle {
    tObject object;
    tVec3f spawn_position;
    tVec3f target_position;
    float speed = 0.f;
    uint32 light_indexes_offset = 0;
  };

  struct VehicleNetworkNode {
    tVec3f position;
    std::vector<VehicleNetworkNode> connected_nodes;
  };

  struct Beacon {
    tObject source_object;
    tObject beacon_1;
    tObject beacon_2;
  };

  struct AutoPlacedObjectList {
    uint16 mesh_index;
    std::vector<uint16> object_ids;
  };

  struct State {
    MeshIds meshes;

    float current_game_time = 0.f;

    Quaternion target_ship_rotation;
    Quaternion target_camera_rotation;
    float target_camera_fov = 45.f;
    float controlled_thrust_duration = 0.f;
    float camera_boost_intensity = 0.f;
    float camera_up_distance = 500.f;

    FlightMode flight_mode = FlightMode::MANUAL_CONTROL;
    AutoDockStage auto_dock_stage;

    tObject docking_target;
    tVec3f docking_position;

    Quaternion initial_approach_camera_rotation;
    Quaternion initial_docking_ship_rotation;
    float initial_approach_camera_distance;
    float initial_approach_ship_distance;
    float initial_docking_ship_distance;

    tVec3f retrograde_direction;
    // @todo make these an orthonormal basis
    tVec3f view_forward_direction;
    tVec3f view_up_direction;

    tVec3f ship_position;
    tVec3f ship_velocity;
    float camera_roll_speed = 0.f;
    float camera_yaw_speed = 0.f;
    float ship_rotate_to_target_speed = 0.f;
    float ship_pitch_factor = 0.f;
    float ship_camera_distance = 1800.f;
    float ship_camera_distance_target = 1800.f;
    float jets_intensity = 0.f;

    OrthonormalBasis ship_rotation_basis;
    OrthonormalBasis ship_velocity_basis;

    TargetStats target_stats;
    std::vector<TargetTracker> on_screen_target_trackers;

    uint8 flight_arrow_cycle_step = 0;
    float flight_path_spawn_distance_remaining = 100.f;
    std::vector<FlightPathNode> incoming_flight_path;

    std::vector<Beacon> beacons;

    std::vector<uint32> gas_flare_light_indexes;
    std::vector<uint32> blinking_light_indexes;

    std::vector<VehicleNetworkNode> vehicle_network;
    std::vector<BackgroundVehicle> vehicles;

    FlightSystem flight_system = FlightSystem::DRONE;
    bool is_piloting_vehicle = false;
    // @todo this will eventually have to be a struct
    // representing all parts of the vehicle + other properties
    tObject piloted_vehicle;
    float piloting_start_time = 0.f;

    tVec2f flight_reticle_offset;
    tVec2f flight_target_reticle_offset;
    float flight_reticle_rotation = 0.f;

    bool photo_mode = false;

    // @todo move to hud_system.cpp
    struct {
      tUIElement* reticle = nullptr;
      tUIElement* dot = nullptr;
      tUIElement* target_indicator = nullptr;
      tUIElement* target_focus = nullptr;
      tUIElement* zone_target_indicator = nullptr;
      tUIElement* selected_target_corner = nullptr;
      tUIElement* selected_target_center = nullptr;

      tUIText* cascadia_mono_20 = nullptr;
      tUIText* cascadia_mono_26 = nullptr;
      tUIText* cascadia_mono_32 = nullptr;
    } ui;

    std::vector<AutoPlacedObjectList> auto_placed_object_lists;

    // @todo dev mode only
    bool is_editor_active = false;
  };
}