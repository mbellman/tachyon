#pragma once

#include "astro/entity_behaviors/behavior.h"
#include "astro/astrolabe.h"

namespace astro {
  behavior SmallBird {
    addMeshes() {
      meshes.small_bird_placeholder = MODEL_MESH("./astro/3d_models/small_bird/placeholder.obj", 500);
      meshes.small_bird_body = MODEL_MESH("./astro/3d_models/small_bird/body.obj", 500);
      meshes.small_bird_head = MODEL_MESH("./astro/3d_models/small_bird/head.obj", 500);
      meshes.small_bird_wings = MODEL_MESH("./astro/3d_models/small_bird/wings.obj", 500);

      mesh(meshes.small_bird_placeholder).shadow_cascade_ceiling = 1;
      mesh(meshes.small_bird_body).shadow_cascade_ceiling = 1;
      mesh(meshes.small_bird_head).shadow_cascade_ceiling = 1;
      mesh(meshes.small_bird_wings).shadow_cascade_ceiling = 1;
    }

    getMeshes() {
      return_meshes({
        meshes.small_bird_body,
        meshes.small_bird_head,
        meshes.small_bird_wings
      });
    }

    getPlaceholderMesh() {
      return meshes.small_bird_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      reset_instances(meshes.small_bird_body);
      reset_instances(meshes.small_bird_head);
      reset_instances(meshes.small_bird_wings);

      for_entities(state.small_birds) {
        auto& entity = state.small_birds[i];

        if (abs(state.player_position.x - entity.visible_position.x) > 15000.f) continue;
        if (abs(state.player_position.z - entity.visible_position.z) > 15000.f) continue;

        // Body
        {
          auto& body = use_instance(meshes.small_bird_body);

          Sync(body, entity);

          commit(body);
        }

        // Head
        {
          auto& head = use_instance(meshes.small_bird_head);

          Sync(head, entity);

          commit(head);
        }

        // Wings
        {
          auto& wings = use_instance(meshes.small_bird_wings);

          Sync(wings, entity);

          commit(wings);
        }
      }
    }
  };
}