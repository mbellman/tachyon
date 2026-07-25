#pragma once

#include "engine/tachyon_aliases.h"
#include "engine/tachyon_types.h"

#include "metro/entities.h"

// @todo move to engine
#define for_range(__low, __high) for (int i = __low; i <= __high; i++)
#define time_since(t) (tachyon->scene.scene_time - (t))
#define get_scene_time() tachyon->scene.scene_time
#define is_moving_left_stick() (tachyon->left_stick.x != 0.f || tachyon->left_stick.y != 0.f)

namespace metro {
  enum BicycleType {
    COMMON_BIKE
  };

  struct Bicycle {
    BicycleType type;

    tColor frame_color;
    tColor wheel_color;
    tColor grips_color;
    tColor saddle_color;

    tVec3f position;
    tVec3f visual_position;
    float speed = 0.f;
    float pedal_speed = 0.f;
    tVec3f facing_direction;

    tVec3f front_wheel_position;
    tVec3f back_wheel_position;
    tVec3f front_wheel_force;
    tVec3f back_wheel_force;

    float steering_angle = 0.f;
    float leaning_angle = 0.f;
    float rocking_factor = 0.f;
    float pitch = 0.f;
    float pedal_revolution = 0.f;
    float wheel_revolution = 0.f;

    int32 id = -1;
    Quaternion computed_rotation;
    Quaternion visual_rotation;
  };

  struct CollisionPlane {
    // Plane corners
    tVec3f p1, p2, p3, p4;
    // Plane tangents
    tVec3f t1, t2, t3, t4;
    tVec3f normal;
  };

  struct MeshIds {
    uint16
      // @temporary
      dev_cube,
      debug_sphere,
      debug_ring,

      // Common bike
      common_frame,
      common_fork,
      common_handlebars,
      common_grips,
      common_seatpost,
      common_saddle,
      common_crank,
      common_pedal,
      common_spokes,
      common_wheel,

      // Static entities
      ramp,

      // Interactive entities
      vending_machine

      ;
  };

  struct State {
    float dt;

    MeshIds meshes;

    tVec3f player_position;

    int32 player_bike_id = -1;

    std::vector<Bicycle> bicycles;
    std::vector<StaticEntity> ramps;
    std::vector<InteractiveEntity> vending_machines;

    std::vector<CollisionPlane> floor_collision_planes;
  };
}