#pragma once

#include "engine/tachyon.h"

namespace Cosmodrone {
  struct MeshIds {
    uint32
      // Ship parts
      hull,
      streams,
      thrusters,
      trim,

      // Celestial bodies
      planet,

      // Debug meshes
      sphere,
      cube;
  };

  struct OrthonormalBasis {
    tVec3f forward = tVec3f(0, 0, -1.f);
    tVec3f up = tVec3f(0, 1.f, 0);
    tVec3f sideways = tVec3f(1.f, 0, 0);
  };

  enum FlightMode {
    MANUAL_CONTROL,
    AUTO_RETROGRADE
  };

  struct State {
    MeshIds meshes;

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
  };
}