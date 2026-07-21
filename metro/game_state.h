#pragma once

#include "engine/tachyon_aliases.h"
#include "engine/tachyon_types.h"

// @todo move to engine
#define time_since(t) (tachyon->scene.scene_time - (t))
#define get_scene_time() tachyon->scene.scene_time
#define is_moving_left_stick() (tachyon->left_stick.x != 0.f || tachyon->left_stick.y != 0.f)

namespace metro {
  // @incomplete
  struct Bicycle {
    tVec3f frame_color;
    tVec3f wheel_color;
    tVec3f handles_color;
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