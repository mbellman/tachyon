#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior HostaBush {
    addMeshes() {
      meshes.hosta_placeholder = MODEL_MESH("./astro/3d_models/hosta_bush/placeholder.obj", 500);
      meshes.hosta_leaves = MODEL_MESH("./astro/3d_models/hosta_bush/leaves.obj", 500);
      meshes.hosta_stalks = MODEL_MESH("./astro/3d_models/hosta_bush/stalks.obj", 500);
      meshes.hosta_flowers = MODEL_MESH("./astro/3d_models/hosta_bush/flowers.obj", 500);

      mesh(meshes.hosta_leaves).type = FOLIAGE_MESH;
      mesh(meshes.hosta_stalks).type = FOLIAGE_MESH;
      mesh(meshes.hosta_flowers).type = FOLIAGE_MESH;
    }

    getMeshes() {
      return_meshes({
        meshes.hosta_leaves,
        meshes.hosta_stalks,
        meshes.hosta_flowers
      });
    }

    getPlaceholderMesh() {
      return meshes.hosta_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      reset_instances(meshes.hosta_leaves);
      reset_instances(meshes.hosta_stalks);
      reset_instances(meshes.hosta_flowers);

      for (auto& entity : state.hosta_bushes) {
        if (!IsDuringActiveTime(entity, state)) continue;
        if (!IsInRangeX(entity, state, 25000.f)) continue;
        if (!IsInRangeZ(entity, state, 25000.f)) continue;

        // Leaves
        {
          auto& leaves = use_instance(meshes.hosta_leaves);

          Sync(leaves, entity);

          leaves.color = tVec3f(0.1f, 0.5f, 0.2f);
          leaves.material = tVec4f(0.8f, 0, 0, 0.4f);

          commit(leaves);
        }

        // Stalks
        {
          auto& stalks = use_instance(meshes.hosta_stalks);

          Sync(stalks, entity);

          stalks.scale *= 0.5f;
          stalks.color = tVec3f(0.14f, 0.3f, 0.07f);
          stalks.material = tVec4f(0.4f, 0, 0, 0.4f);

          commit(stalks);
        }

        // Flowers
        {
          auto& flowers = use_instance(meshes.hosta_flowers);

          Sync(flowers, entity);

          flowers.scale *= 0.5f;

          commit(flowers);
        }
      }
    }
  };
}
