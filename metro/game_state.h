#pragma once

#include "engine/tachyon_aliases.h"
#include "engine/tachyon_types.h"

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
    float speed = 0.f;
    float pedal_speed = 0.f;
    tVec3f facing_direction;

    float steering_angle = 0.f;
    float leaning_angle = 0.f;
    float pedal_revolution = 0.f;
    float wheel_revolution = 0.f;

    int32 id = -1;
    Quaternion computed_rotation;
  };

  struct MeshIds {
    uint16
      // @temporary
      cube,
      bicycle,

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
      common_wheel

      ;
  };

  struct State {
    float dt;

    MeshIds meshes;

    std::vector<Bicycle> bicycles;
    int32 player_bike_id = -1;
    tVec3f player_position;
  };
}