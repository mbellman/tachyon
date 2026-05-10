
#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior LilyPadCluster {
    addMeshes() {
      meshes.lily_pad_cluster_placeholder = MODEL_MESH("./astro/3d_models/lily_pad_cluster/placeholder.obj", 500);
      meshes.lily_pad_cluster = MODEL_MESH("./astro/3d_models/lily_pad_cluster/cluster.obj", 500);
      meshes.lily_pad_flower = MODEL_MESH("./astro/3d_models/lily_pad_cluster/flower.obj", 500);

      mesh(meshes.lily_pad_cluster_placeholder).shadow_cascade_ceiling = 0;
      mesh(meshes.lily_pad_cluster).shadow_cascade_ceiling = 0;
      mesh(meshes.lily_pad_flower).shadow_cascade_ceiling = 2;

      mesh(meshes.lily_pad_cluster).type = FOLIAGE_MESH;
      mesh(meshes.lily_pad_flower).type = FOLIAGE_MESH;
    }

    getMeshes() {
      return_meshes({
        meshes.lily_pad_cluster,
        meshes.lily_pad_flower
      });
    }

    getPlaceholderMesh() {
      return meshes.lily_pad_cluster_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      reset_instances(meshes.lily_pad_cluster);
      reset_instances(meshes.lily_pad_flower);

      for (auto& entity : state.lily_pad_clusters) {
        if (abs(state.player_position.x - entity.position.x) > 25000.f) continue;
        if (abs(state.player_position.z - entity.position.z) > 25000.f) continue;

        // Lily pads
        {
          auto& cluster = use_instance(meshes.lily_pad_cluster);

          Sync(cluster, entity);

          cluster.material = tVec4f(0.5f, 0, 1.f, 0.4f);

          commit(cluster);
        }

        // Flower
        {
          auto& flower = use_instance(meshes.lily_pad_flower);
          float angle = fmodf(entity.position.x, t_TAU);

          Sync(flower, entity);

          flower.rotation = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), angle);
          flower.color = tVec4f(1.f, 0.7f, 0.8f, 0.1f);
          flower.material = tVec4f(1.f, 0, 0, 1.f);

          commit(flower);
        }
      }
    }
  };
}
