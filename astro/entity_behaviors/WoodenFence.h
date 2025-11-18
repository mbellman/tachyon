#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior WoodenFence {
    addMeshes() {
      meshes.wooden_fence_placeholder = MODEL_MESH("./astro/3d_models/wooden_fence/placeholder.obj", 500);
      meshes.wooden_fence_post = MODEL_MESH("./astro/3d_models/wooden_fence/post.obj", 1500);
      meshes.wooden_fence_beam = MODEL_MESH("./astro/3d_models/wooden_fence/beam.obj", 1000);
    }

    getMeshes() {
      return_meshes({
        // 3 posts per fence section
        // @todo make dynamic (?)
        meshes.wooden_fence_post,
        meshes.wooden_fence_post,
        meshes.wooden_fence_post,

        // 2 beams per fence section
        // @todo make dynamic (?)
        meshes.wooden_fence_beam,
        meshes.wooden_fence_beam
      });
    }

    getPlaceholderMesh() {
      return meshes.wooden_fence_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      for_entities(state.wooden_fences) {
        auto& entity = state.wooden_fences[i];

        if (abs(state.player_position.x - entity.position.x) > 25000.f) continue;
        if (abs(state.player_position.z - entity.position.z) > 25000.f) continue;

        // Posts
        {
          uint16 post_index = i * 3;
          tVec3f offset = entity.orientation.toMatrix4f() * tVec3f(1.5f, 0, 0);

          for (int index = 0; index < 3; index++) {
            auto& post = objects(meshes.wooden_fence_post)[post_index + (uint16)index];

            post.position = entity.position;
            post.position.y += entity.scale.y * 0.75f;
            post.position += offset * float(index - 1) * 1000.f;

            post.scale = entity.scale;
            post.rotation = entity.orientation;
            post.color = entity.tint;

            commit(post);
          }
        }

        // Beams
        {
          uint16 beam_index = i * 2;

          auto& beam = objects(meshes.wooden_fence_beam)[beam_index];
          auto& beam_2 = objects(meshes.wooden_fence_beam)[beam_index + 1];

          beam.position = beam_2.position = entity.position;
          beam.scale = beam_2.scale = entity.scale;
          beam.rotation = beam_2.rotation = entity.orientation;
          beam.color = beam_2.color = entity.tint;

          beam.position.y += 1.1f * entity.scale.y;
          beam_2.position.y += 0.4f * entity.scale.y;

          commit(beam);
          commit(beam_2);
        }

        // Collision
        entity.visible_position = entity.position;
        entity.visible_scale = entity.scale;
        entity.visible_rotation = entity.orientation;
      }
    }
  };
}