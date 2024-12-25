#pragma once

#include "engine/tachyon.h"

namespace Cosmodrone {
  struct MeshIds {
    uint16
      // Ship parts
      hull,
      streams,
      thrusters,
      trim,

      // NPC drone parts
      npc_drone_1,

      // Spawn meshes
      zone_target,

      // Station parts
      antenna_1,
      antenna_2,
      antenna_3,
      radio_tower_1,
      module_1,
      module_2,
      habitation_1,
      habitation_2,
      habitation_3,
      habitation_4,
      silo_2,
      silo_3,
      silo_4,
      silo_5,
      silo_6,
      torus_1,
      elevator_torus_1,
      station_torus_1,
      station_torus_2,
      station_torus_3,
      station_base,
      solar_panel_1,
      solar_panel_2,
      girder_1,
      girder_2,
      girder_3,
      girder_4,
      girder_5,
      girder_6,
      grate_1,
      grate_2,
      track_1,
      light_1,
      light_2,
      light_3,

      // Mega parts
      mega_girder_1,

      // Large objects
      spire_fortress,
      gate_tower_1,
      arch_1,
      upper_facility,

      // Generated large objects
      arch_1_body,
      arch_1_frame,
      arch_1_details,

      // Generated station parts
      antenna_2_frame,
      antenna_2_receivers,
      silo_3_body,
      silo_3_frame,
      silo_6_body,
      silo_6_frame,
      silo_6_pipes,
      girder_4_core,
      girder_4_frame,
      girder_6_core,
      girder_6_frame,
      habitation_1_core,
      habitation_1_frame,
      habitation_1_insulation,
      habitation_2_body,
      habitation_2_frame,
      habitation_3_core,
      habitation_3_frame,
      habitation_4_body,
      habitation_4_core,
      habitation_4_frame,
      habitation_4_panels,
      habitation_4_lights,
      module_2_core,
      module_2_frame,
      solar_panel_2_cells,
      solar_panel_2_frame,
      elevator_torus_1_frame,
      station_torus_2_body,
      station_torus_2_supports,
      station_torus_2_frame,
      station_torus_3_body,
      station_torus_3_frame,
      station_torus_3_lights,
      light_1_base,
      light_1_bulb,
      light_2_base,
      light_2_bulb,
      light_3_base,
      light_3_bulb,

      // Target inspector meshes
      antenna_3_wireframe,

      // Entities
      elevator_car_1,
      elevator_car_1_frame,
      gas_flare_1_spawn,
      gas_flare_1,

      // HUD,
      hud_wedge,
      hud_flight_arrow,

      // Background elements
      planet,
      space_elevator,

      // Volumetrics
      earth_atmosphere,

      // Debug meshes
      sphere,
      cube,
      editor_guideline,
      editor_position,
      editor_rotation,
      editor_scale;
  };

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

  struct TargetTracker {
    tObject object;
    float activated_time = 0.f;
    float selected_time = 0.f;
    float deselected_time = 0.f;
    float deactivated_time = 0.f;

    int32 screen_x = 0;
    int32 screen_y = 0;
    float center_distance = 0.f;
  };

  enum AutoDockStage {
    APPROACH_DECELERATION,
    APPROACH_ALIGNMENT,
    APPROACH,
    DOCKING,
    DOCKED
  };

  struct FlightPathNode {
    tVec3f position;
    float distance;
    tVec3f spawn_position;
    float spawn_distance;
  };

  struct Vehicle {
    tObject object;
    tVec3f position;
    tVec3f direction;
    float speed;
    uint32 light_index;
  };

  struct State {
    MeshIds meshes;

    float current_game_time = 0.f;

    Quaternion target_ship_rotation;
    Quaternion target_camera_rotation;
    float target_camera_fov = 45.f;

    FlightMode flight_mode = FlightMode::MANUAL_CONTROL;

    tVec3f retrograde_direction;

    // @todo make these an orthonormal basis
    tVec3f view_forward_direction;
    tVec3f view_up_direction;

    tVec3f ship_position;
    tVec3f ship_velocity;
    float camera_roll_speed = 0.f;
    float ship_rotate_to_target_speed = 0.f;
    float ship_pitch_factor = 0.f;
    float ship_camera_distance = 1800.f;
    float ship_camera_distance_target = 1800.f;

    OrthonormalBasis ship_rotation_basis;
    OrthonormalBasis ship_velocity_basis;

    std::vector<TargetTracker> on_screen_target_trackers;
    AutoDockStage auto_dock_stage;
    tObject docking_target;
    tVec3f docking_position;

    uint8 flight_arrow_cycle_step = 0;
    float flight_path_spawn_distance_remaining = 100.f;
    std::vector<FlightPathNode> incoming_flight_path;

    std::vector<uint32> gas_flare_light_indexes;

    std::vector<Vehicle> vehicles;

    // @todo move to UI/UISystem
    struct {
      tUIElement* target_indicator = nullptr;
      tUIElement* zone_target_indicator = nullptr;
      tUIElement* selected_target_corner = nullptr;
      tUIElement* selected_target_center = nullptr;

      tUIText* cascadia_mono_26 = nullptr;
      tUIText* cascadia_mono_32 = nullptr;
    } ui;

    // @todo dev mode only
    bool is_editor_active = false;
  };
}