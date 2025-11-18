#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior WoodenFence {
    addMeshes() {
      meshes.wooden_fence_placeholder = MODEL_MESH("./astro/3d_models/wooden_fence/placeholder.obj", 500);
      meshes.wooden_fence_post = MODEL_MESH("./astro/3d_models/wooden_fence/post.obj", 2500);
      meshes.wooden_fence_beam = MODEL_MESH("./astro/3d_models/wooden_fence/beam.obj", 1000);
    }

    getMeshes() {
      return_meshes({
        // 5 posts per fence section
        meshes.wooden_fence_post,
        meshes.wooden_fence_post,
        meshes.wooden_fence_post,
        meshes.wooden_fence_post,
        meshes.wooden_fence_post,

        // 2 beams per fence section
        meshes.wooden_fence_beam,
        meshes.wooden_fence_beam
      });
    }

    getPlaceholderMesh() {
      return meshes.wooden_fence_placeholder;
    }

    timeEvolve() {
      profile("  WoodenFence::timeEvolve()");

      auto& meshes = state.meshes;

      Quaternion tilt_rotations[] = {
        Quaternion::fromAxisAngle(tVec3f(0, 0, 1.f), 0.06f),
        Quaternion::fromAxisAngle(tVec3f(0, 0, 1.f), -0.11f),
        Quaternion::fromAxisAngle(tVec3f(0, 0, 1.f), -0.07f),
      };

      Quaternion yaw_90 = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), t_HALF_PI);

      for_entities(state.wooden_fences) {
        auto& entity = state.wooden_fences[i];

        if (abs(state.player_position.x - entity.position.x) > 25000.f) continue;
        if (abs(state.player_position.z - entity.position.z) > 25000.f) continue;

        float age = state.astro_time - entity.astro_start_time;
        if (age < 0.f) age = 0.f;

        // Posts
        {
          uint16 post_index = i * 5;
          tVec3f x_offset = entity.orientation.toMatrix4f() * tVec3f(1.f, 0, 0);
          tVec3f z_offset = entity.orientation.toMatrix4f() * tVec3f(0, 0, 1.f);

          for (int index = 0; index < 5; index++) {
            auto& post = objects(meshes.wooden_fence_post)[post_index + (uint16)index];

            post.position = entity.position;
            post.position += x_offset * (float(index) - 2.f) * 900.f;
            post.position.y += entity.scale.y * 0.75f;
            post.position.y -= fmodf(abs(post.position.x), 200.f);

            post.rotation = entity.orientation;
            post.scale = entity.scale;

            // Middle posts
            if (index == 1 || index == 2 || index == 3) {
              // Offset behind beams
              post.position -= z_offset * 200.f;

              // Tilt
              Quaternion tilt_rotation = tilt_rotations[int(post.position.x) % 3];

              post.rotation = entity.orientation * tilt_rotation;

              // Make thinner
              post.scale.x *= 0.6f;
              post.scale.z *= 0.6f;
            }

            post.color = tVec3f(1.f, 0.8f, 0.4f);

            // @todo gradually reveal fence structure
            if (age == 0.f) post.scale = tVec3f(0.f);

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
          beam.color = beam_2.color = tVec3f(1.f, 0.8f, 0.4f);

          // Top beam
          beam.position.y += 1.f * entity.scale.y;

          // Bottom beam
          beam_2.position.y += 0.4f * entity.scale.y;

          // @todo gradually reveal fence structure
          if (age == 0.f) {
            beam.scale = tVec3f(0.f);
            beam_2.scale = tVec3f(0.f);
          }

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