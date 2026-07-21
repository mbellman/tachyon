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
    tColor handles_color;
    tColor seat_color;

    tVec3f position;
    Quaternion rotation;
  };

  struct MeshIds {
    uint16
      // @temporary
      cube,
      bicycle,

      // Common bike
      common_frame,
      common_skeleton,
      common_handles,
      common_seat,
      common_wheels

      ;
  };

  struct State {
    float dt;

    MeshIds meshes;

    std::vector<Bicycle> bicycles;
  };
}