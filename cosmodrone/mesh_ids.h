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
      station_drone_1,
      flying_ship_1,

      // Marker/spawn meshes
      zone_target,
      vehicle_target,

      // Station parts
      antenna_1,
      antenna_2,
      antenna_3,
      antenna_4,
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
      station_torus_4,
      station_platform_1,
      station_cylinder_1,
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
      charge_pad,

      // Mega parts
      mega_girder_1,

      // Background elements
      background_ship_1,

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
      antenna_4_base,
      antenna_4_dish,
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
      light_1_base,
      light_1_bulb,
      light_2_base,
      light_2_bulb,
      light_3_base,
      light_3_bulb,
      track_1_frame,

      // Procedural meshes
      procedural_elevator_car,
      procedural_track_1,

      // HUD wireframe meshes
      drone_wireframe,
      antenna_3_wireframe,

      // Entities
      elevator_car_1,
      elevator_car_1_frame,
      gas_flare_1_spawn,
      gas_flare_1,

      // HUD,
      hud_flight_arrow,
      beacon,

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
}