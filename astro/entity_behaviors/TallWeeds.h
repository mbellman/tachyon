#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior TallWeeds {
    addMeshes() {
      meshes.tall_weeds_placeholder = MODEL_MESH("./astro/3d_models/tall_weeds/placeholder.obj", 500);
      meshes.tall_weeds = MODEL_MESH("./astro/3d_models/tall_weeds/weeds.obj", 500);

      mesh(meshes.tall_weeds_placeholder).shadow_cascade_ceiling = 2;
      mesh(meshes.tall_weeds).shadow_cascade_ceiling = 2;

      mesh(meshes.tall_weeds).type = FOLIAGE_MESH;
    }

    getMeshes() {
      return_meshes({
        meshes.tall_weeds
      });
    }

    getPlaceholderMesh() {
      return meshes.tall_weeds_placeholder;
    }

    timeEvolve() {
      profile("  TallWeeds::timeEvolve()");

      auto& meshes = state.meshes;

      reset_instances(meshes.tall_weeds);

      Quaternion rotation_cycle[4] = {
        Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), 0.f),
        Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), t_HALF_PI),
        Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), t_PI),
        Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), t_PI + t_HALF_PI),
      };

      for_entities(state.tall_weeds) {
        auto& entity = state.tall_weeds[i];

        if (abs(state.player_position.x - entity.position.x) > 20000.f) continue;
        if (abs(state.player_position.z - entity.position.z) > 20000.f) continue;

        float life_progress = GetLivingEntityProgress(state, entity, 100.f);

        if (life_progress == 0.f || life_progress == 1.f) continue;

        auto& weeds = use_instance(meshes.tall_weeds);

        Sync(weeds, entity);
        float growth_factor = sqrtf(sinf(life_progress * t_PI));

        weeds.scale *= growth_factor;
        weeds.color = tVec4f(0.1f, 0.3f, 0.1f, 0.2f);
        weeds.material = tVec4f(0.4f, 0, 0, 0.2f);

        commit(weeds);
      }
    }
  }
}