#pragma once

#include "engine/tachyon_aliases.h"

namespace Cosmodrone {
  struct MeshIds {
    uint16
      // Ship parts
      hull,
      streams,
      thrusters,
      trim,
      jets,

      // NPC drones/ships
      station_drone,
      flying_ship_1,

      // Marker/spawn meshes
      zone_target,
      vehicle_target,

      // Major station pieces
      platform,
      carrier,

      // Station parts
      antenna_1,
      antenna_2,
      antenna_3,
      antenna_4,
      antenna_5,
      machine_1,
      machine_2,
      machine_3,
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
      silo_7,
      silo_8,
      torus_1,
      elevator_torus_1,
      station_torus_1,
      station_torus_2,
      station_torus_3,
      station_torus_4,
      station_platform_1,
      solar_rotator,
      solar_field,
      base_1,
      building_1,
      station_base,
      solar_panel_1,
      solar_panel_2,
      girder_1,
      girder_1b,
      girder_2,
      girder_3,
      girder_4,
      girder_5,
      girder_6,
      beam_1,
      beam_2,
      grate_1,
      grate_2,
      grate_3,
      grate_4,
      track_1,
      light_1,
      light_2,
      light_3,
      light_4,
      charge_pad,
      floater_1,

      // Mega parts
      mega_girder_1,
      mega_girder_2,

      // Background elements
      background_ship_1,

      // Large objects
      gate_tower_1,
      arch_1,

      // Generated large objects
      arch_1_body,
      arch_1_frame,
      arch_1_details,

      // Generated station parts
      antenna_2_frame,
      antenna_2_receivers,
      antenna_4_base,
      antenna_4_dish,
      antenna_5_dish,
      silo_3_body,
      silo_3_frame,
      silo_6_body,
      silo_6_frame,
      silo_6_pipes,
      silo_7_core,
      silo_7_frame,
      silo_8_core,
      silo_8_frame,
      girder_4_core,
      girder_4_frame,
      girder_6_core,
      girder_6_frame,
      beam_2_core,
      beam_2_frame,
      grate_4_frame,
      habitation_1_core,
      habitation_1_frame,
      habitation_1_insulation,
      habitation_2_body,
      habitation_2_frame,
      habitation_3_core,
      habitation_3_frame,
      habitation_3_lights,
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
      station_torus_4_body,
      station_torus_4_frame,
      platform_body,
      platform_torus,
      platform_frame,
      platform_supports,
      solar_rotator_body,
      solar_rotator_frame,
      solar_rotator_panels,
      solar_field_core,
      solar_field_frame,
      solar_field_panels,
      light_1_base,
      light_1_bulb,
      light_2_base,
      light_2_bulb,
      light_3_base,
      light_3_bulb,
      light_4_base,
      light_4_bulb,
      track_1_frame,
      floater_1_core,
      floater_1_base,
      floater_1_frame,
      floater_1_spokes,
      floater_1_panels,

      // Vehicle spawns
      fighter_spawn,
      freight_spawn,

      // Vehicle parts
      station_drone_core,
      station_drone_frame,
      station_drone_rotator,
      station_drone_light,
      fighter_core,
      fighter_frame,
      fighter_dock,
      fighter_guns,
      fighter_thrusters,
      fighter_left_wing_core,
      fighter_left_wing_turrets,
      fighter_right_wing_core,
      fighter_right_wing_turrets,
      fighter_jets,

      // Procedural meshes
      procedural_elevator_car,
      procedural_elevator_car_light,
      procedural_track_1,
      procedural_track_supports_1,
      procedural_track_supports_2,

      // HUD wireframe meshes
      drone_wireframe,
      antenna_3_wireframe,
      floater_1_wireframe,
      station_drone_wireframe,
      fighter_wireframe,
      freight_wireframe,

      // Entities
      elevator_car_1,
      elevator_car_1_frame,
      gas_flare_1_spawn,
      gas_flare_1,

      // HUD,
      hud_flight_arrow,
      hud_flight_curve,
      beacon,

      // Background elements
      earth,
      moon,
      space_elevator, // @todo remove

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
}