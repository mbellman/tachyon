#pragma once

#include "engine/tachyon_aliases.h"
#include "engine/tachyon_types.h"

// @todo move to engine
#define time_since(t) (tachyon->scene.scene_time - (t))
#define get_scene_time() tachyon->scene.scene_time
#define is_moving_left_stick() (tachyon->left_stick.x != 0.f || tachyon->left_stick.y != 0.f)

namespace metro {
  struct MeshIds {
    uint16
      // @temporary
      cube,
      bicycle;
  };

  struct State {
    float dt;

    MeshIds meshes;
  };
}