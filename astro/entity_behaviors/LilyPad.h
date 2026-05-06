#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior LilyPad {
    addMeshes() {
      meshes.lily_pad_placeholder = MODEL_MESH("./astro/3d_models/lily_pad/placeholder.obj", 500);
      meshes.lily_pad = MODEL_MESH("./astro/3d_models/lily_pad/pad.obj", 500);

      mesh(meshes.lily_pad_placeholder).shadow_cascade_ceiling = 0;
      mesh(meshes.lily_pad).shadow_cascade_ceiling = 0;
    }

    getMeshes() {
      return_meshes({
        meshes.lily_pad
      });
    }

    getPlaceholderMesh() {
      return meshes.lily_pad_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      const tVec3f default_color = tVec3f(0.3f, 0.7f, 0.1f);

      for_entities(state.lily_pads) {
        auto& entity = state.lily_pads[i];

        // @todo factor
        if (abs(state.player_position.x - entity.position.x) > 30000.f) continue;
        if (abs(state.player_position.z - entity.position.z) > 30000.f) continue;

        float life_progress = Tachyon_InverseLerp(entity.astro_start_time, entity.astro_end_time, state.astro_time);
        float rotation_angle = 0.2f * sinf(0.8f * get_scene_time() + entity.position.z);

        // Lily pad
        {
          auto& pad = objects(meshes.lily_pad)[i];

          Sync(pad, entity);

          // Bob up and down on the water surface
          pad.position.y += 25.f * sinf(2.5f * get_scene_time() + entity.position.x);
          pad.rotation *= Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), rotation_angle);

          // Grow and shrink over lifetime
          float scale_alpha = sqrtf(sinf(life_progress * t_PI));

          pad.scale *= scale_alpha;

          pad.color = default_color;
          pad.material = tVec4f(0.4f, 0, 0, 0.4f);

          commit(pad);
        }
      }
    }
  };
}