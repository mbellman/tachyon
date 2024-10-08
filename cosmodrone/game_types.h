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

      // station parts
      antenna_1,
      antenna_2,
      module_1,
      habitation_1,
      habitation_2,
      silo_2,
      silo_3,
      silo_4,
      torus_1,
      station_torus_1,
      station_base,
      solar_panel_1,
      solar_panel_2,
      girder_1,
      girder_2,
      girder_3,
      girder_4,
      girder_5,
      track_1,

      // Background elements
      planet,
      space_elevator,

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
    AUTO_RETROGRADE
  };

  struct State {
    MeshIds meshes;

    float current_game_time = 0.f;

    Quaternion target_camera_rotation = Quaternion(1.f, 0, 0, 0);
    FlightMode flight_mode = FlightMode::MANUAL_CONTROL;

    tVec3f view_forward_direction;
    tVec3f view_up_direction;

    tVec3f ship_position;
    tVec3f ship_velocity;
    float camera_roll_speed = 0.f;
    float ship_rotate_to_target_speed = 0.f;

    OrthonormalBasis ship_rotation_basis;
    OrthonormalBasis ship_velocity_basis;

    // @todo dev mode only
    bool is_editor_active = false;
  };
}