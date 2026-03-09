#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior TulipPlant {
    addMeshes() {
      meshes.tulip_plant_placeholder = MODEL_MESH("./astro/3d_models/tulip_plant/placeholder.obj", 500);
      meshes.tulip_plant_leaves = MODEL_MESH("./astro/3d_models/tulip_plant/leaves.obj", 500);
      meshes.tulip_plant_stalk = MODEL_MESH("./astro/3d_models/tulip_plant/stalk.obj", 500);
      meshes.tulip_plant_bulb = MODEL_MESH("./astro/3d_models/tulip_plant/bulb.obj", 500);

      mesh(meshes.tulip_plant_placeholder).shadow_cascade_ceiling = 2;
      mesh(meshes.tulip_plant_leaves).shadow_cascade_ceiling = 2;
      mesh(meshes.tulip_plant_stalk).shadow_cascade_ceiling = 2;
      mesh(meshes.tulip_plant_bulb).shadow_cascade_ceiling = 2;

      mesh(meshes.tulip_plant_leaves).type = FOLIAGE_MESH;
      mesh(meshes.tulip_plant_stalk).type = FOLIAGE_MESH;
      mesh(meshes.tulip_plant_bulb).type = FOLIAGE_MESH;
    }

    getMeshes() {
      return_meshes({
        meshes.tulip_plant_leaves,
        meshes.tulip_plant_stalk,
        meshes.tulip_plant_bulb
      });
    }

    getPlaceholderMesh() {
      return meshes.tulip_plant_placeholder;
    }

    // @todo optimize rendering
    // @todo one or two flower stalks
    // @todo alternate bulb coloration
    timeEvolve() {
      profile("  TulipPlant::timeEvolve()");

      auto& meshes = state.meshes;

      // reset_instances(meshes.tall_grass);

      for_entities(state.tulip_plants) {
        auto& entity = state.tulip_plants[i];

        // if (abs(state.player_position.x - entity.position.x) > 20000.f) continue;
        // if (abs(state.player_position.z - entity.position.z) > 20000.f) continue;

        // auto& grass = use_instance(meshes.tall_grass);

        // Leaves
        {
          auto& leaves = objects(meshes.tulip_plant_leaves)[i];

          Sync(leaves, entity);

          leaves.color = tVec3f(0.4f, 0.8f, 0.2f);
          leaves.material = tVec4f(0.8f, 0, 0, 0.2f);

          commit(leaves);
        }

        // Stalk
        {
          auto& stalk = objects(meshes.tulip_plant_stalk)[i];

          Sync(stalk, entity);

          stalk.material = tVec4f(0.8f, 0, 0, 0.2f);

          commit(stalk);
        }

        // Bulb
        {
          auto& bulb = objects(meshes.tulip_plant_bulb)[i];

          Sync(bulb, entity);

          bulb.position.y += entity.scale.y * 1.4f;
          bulb.scale *= 0.35f;
          bulb.color = tVec4f(1.f, 0.4f, 0.7f, 0.2f);
          bulb.material = tVec4f(0.9f, 0, 0, 1.f);

          commit(bulb);
        }
      }
    }
  }
}