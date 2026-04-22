#pragma once

#include "astro/entity_behaviors/behavior.h"

namespace astro {
  behavior BirdGate {
    static Quaternion Y_FLIP = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), t_PI);

    static void Open(GameEntity& entity, const float scene_time) {
      entity.did_activate = true;
      entity.game_activation_time = scene_time;
    }

    addMeshes() {
      meshes.bird_gate_placeholder = MODEL_MESH("./astro/3d_models/bird_gate/placeholder.obj", 500);
      meshes.bird_gate = MODEL_MESH("./astro/3d_models/bird_gate/gate_half.obj", 500);
    }

    getMeshes() {
      return_meshes({
        // Left half
        meshes.bird_gate,
        // Right half
        meshes.bird_gate
      });
    }

    getPlaceholderMesh() {
      return meshes.bird_gate_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;
      float player_speed = state.player_velocity.magnitude();

      reset_instances(meshes.bird_gate);

      for_entities(state.bird_gates) {
        auto& entity = state.bird_gates[i];

        if (abs(state.player_position.x - entity.position.x) > 25000.f) continue;
        if (abs(state.player_position.z - entity.position.z) > 25000.f) continue;

        // @todo fix the model's orientation so this is actually getLeftDirection()
        tVec3f side_direction = entity.orientation.getDirection();

        // Interaction
        {

          if (!entity.did_activate) {
            auto proximity = GetEntityProximity(entity, state);

            if (
              proximity.distance < 4000.f &&
              proximity.facing_dot > 0.f &&
              // @temporary @todo determine whether we're on the "back" side of the gate
              state.player_position.z < entity.position.z
            ) {
              UISystem::ShowTransientDialogue(tachyon, state, "[X] Open");

              if (did_press_key(tKey::CONTROLLER_A) && player_speed < 200.f) {
                Open(entity, get_scene_time());

                if (state.astro_time == astro_time_periods.present) {
                  Sfx::PlaySound(SFX_GATE_OPEN, 0.5f);
                } else {
                  // @todo
                }
              }
            }
          }
        }

        // Opening the gate
        float open_alpha = 0.f;

        {
          if (entity.did_activate) {
            open_alpha = time_since(entity.game_activation_time) / 4.f;
            if (open_alpha > 1.f) open_alpha = 1.f;
            open_alpha = Tachyon_EaseInOutf(open_alpha);
          }
        }

        // Gate (left)
        {
          auto& left_gate = use_instance(meshes.bird_gate);

          Sync(left_gate, entity);

          left_gate.position -= side_direction * entity.scale.z * 0.575f;
          left_gate.color = tVec3f(1.f, 0.9f, 0.2f);
          left_gate.material = tVec4f(0.f, 1.f, 0, 0.6f);

          // Handle opening animation
          left_gate.rotation = Quaternion::slerp(
            left_gate.rotation,
            left_gate.rotation * Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), -1.2f),
            open_alpha
          );

          commit(left_gate);
        }

        // Gate (right)
        {
          auto& right_gate = use_instance(meshes.bird_gate);

          Sync(right_gate, entity);

          right_gate.position += side_direction * entity.scale.z * 0.575f;
          right_gate.color = tVec3f(1.f, 0.9f, 0.2f);
          right_gate.material = tVec4f(0.f, 1.f, 0, 0.6f);
          right_gate.rotation *= Y_FLIP;

          // Handle opening animation
          right_gate.rotation = Quaternion::slerp(
            right_gate.rotation,
            right_gate.rotation * Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), 1.2f),
            open_alpha
          );

          commit(right_gate);
        }

        // Collision
        entity.visible_position = entity.position;
        entity.visible_rotation = entity.orientation;
        entity.visible_scale = entity.scale * tVec3f(0.4f, 1.f, 1.4f);
      }
    }
  };
}