#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior TallGrass {
    addMeshes() {
      meshes.tall_grass_placeholder = MODEL_MESH("./astro/3d_models/tall_grass/placeholder.obj", 500);
      meshes.tall_grass = MODEL_MESH("./astro/3d_models/tall_grass/grass.obj", 500);

      mesh(meshes.tall_grass_placeholder).shadow_cascade_ceiling = 2;
      mesh(meshes.tall_grass).shadow_cascade_ceiling = 2;

      mesh(meshes.tall_grass).type = GRASS_MESH;
    }

    getMeshes() {
      return_meshes({
        meshes.tall_grass
      });
    }

    getPlaceholderMesh() {
      return meshes.tall_grass_placeholder;
    }

    timeEvolve() {
      profile("  TallGrass::timeEvolve()");

      auto& meshes = state.meshes;

      reset_instances(meshes.tall_grass);

      Quaternion rotation_cycle[4] = {
        Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), 0.f),
        Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), t_HALF_PI),
        Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), t_PI),
        Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), t_PI + t_HALF_PI),
      };

      for_entities(state.tall_grasses) {
        auto& entity = state.tall_grasses[i];

        if (abs(state.player_position.x - entity.position.x) > 20000.f) continue;
        if (abs(state.player_position.z - entity.position.z) > 20000.f) continue;

        auto& grass = use_instance(meshes.tall_grass);

        Sync(grass, entity);

        // Time-dependent variation
        {
          int cycle_index = int(abs(state.astro_time * 0.05f)) % 4;

          float x = (state.astro_time * 0.5f) + t_HALF_PI;
          float growth_factor = 0.5f + 0.5f * sinf(x);

          grass.rotation *= rotation_cycle[cycle_index];
          grass.scale = entity.scale * growth_factor;
          grass.material = tVec4f(0.4f, 0, 0, 0.2f);
        }

        commit(grass);
      }
    }
  }
}