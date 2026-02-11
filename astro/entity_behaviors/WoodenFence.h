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

      uint16 post_index = 0;
      uint16 beam_index = 0;

      for_entities(state.wooden_fences) {
        auto& entity = state.wooden_fences[i];

        if (abs(state.player_position.x - entity.position.x) > 25000.f) continue;
        if (abs(state.player_position.z - entity.position.z) > 25000.f) continue;

        float age = state.astro_time - entity.astro_start_time;
        if (age < 0.f) age = 0.f;

        // Posts
        {
          tVec3f x_offset = entity.orientation.toMatrix4f() * tVec3f(1.f, 0, 0);
          tVec3f z_offset = entity.orientation.toMatrix4f() * tVec3f(0, 0, 1.f);

          for (int index = 0; index < 5; index++) {
            auto& post = objects(meshes.wooden_fence_post)[post_index++];

            post.position = entity.position;
            post.position += x_offset * (float(index) - 2.f) * 900.f;
            post.position.y += entity.scale.y * 0.75f;
            post.position.y -= fmodf(abs(post.position.x), 200.f);

            post.rotation = entity.orientation;
            post.scale = entity.scale;

            // Middle posts
            if (index > 0 && index < 4) {
              // Offset behind beams
              post.position -= z_offset * 150.f;
              post.position.y -= 200.f;

              // Tilt
              Quaternion tilt_rotation = tilt_rotations[int(abs(post.position.x)) % 3];

              post.rotation = entity.orientation * tilt_rotation;

              // Make thinner
              post.scale.x *= 0.6f;
              post.scale.z *= 0.4f;
            }

            // Reveal posts only after a bit of time
            if (index == 0 && age < 2.f) post.scale = tVec3f(0.f);
            if (index == 1 && age < 10.f) post.scale = tVec3f(0.f);
            if (index == 2 && age < 15.f) post.scale = tVec3f(0.f);
            if (index == 3 && age < 12.f) post.scale = tVec3f(0.f);

            if (age == 0.f) post.scale = tVec3f(0.f);

            post.color = tVec3f(0.7f, 0.5f, 0.2f);
            post.material = tVec4f(0.9f, 0, 0, 0.4f);

            commit(post);
          }
        }

        // Beams
        {
          auto& top_beam = objects(meshes.wooden_fence_beam)[beam_index++];
          auto& bottom_beam = objects(meshes.wooden_fence_beam)[beam_index++];

          top_beam.position = bottom_beam.position = entity.position;
          top_beam.scale = bottom_beam.scale = entity.scale;
          top_beam.rotation = bottom_beam.rotation = entity.orientation;
          top_beam.color = bottom_beam.color = tVec3f(0.7f, 0.5f, 0.2f);
          top_beam.material = bottom_beam.material = tVec4f(0.9f, 0, 0, 0.4f);

          top_beam.position.y += 1.f * entity.scale.y;
          bottom_beam.position.y += 0.4f * entity.scale.y;

          if (age == 0.f) {
            top_beam.scale = tVec3f(0.f);
            bottom_beam.scale = tVec3f(0.f);
          }

          // Reveal the beams in stages
          if (age < 6.f) top_beam.scale = tVec3f(0.f);
          if (age < 3.f) bottom_beam.scale = tVec3f(0.f);

          commit(top_beam);
          commit(bottom_beam);
        }

        entity.visible_position = entity.position;
        entity.visible_rotation = entity.orientation;

        // Collision
        entity.visible_scale = entity.scale * tVec3f(1.3f, 1.f, 0.4f);
      }

      mesh(meshes.wooden_fence_post).lod_1.instance_count = post_index;
      mesh(meshes.wooden_fence_beam).lod_1.instance_count = beam_index;
    }
  };
}