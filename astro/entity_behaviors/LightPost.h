#pragma once

#include "astro/entity_behaviors/behavior.h"
#include "astro/sfx.h"
#include "astro/entity_manager.h"

namespace astro {
  static bool IsResponder(const GameEntity& entity) {
    return entity.associated_entity_record.id != -1;
  }

  static const GameEntity* GetFinalAssociatedEntity(State& state, const GameEntity& entity) {
    if (entity.associated_entity_record.id != -1) {
      auto& associated_entity = *EntityManager::FindEntity(state, entity.associated_entity_record);

      return GetFinalAssociatedEntity(state, associated_entity);
    } else {
      return &entity;
    }
  }

  static bool IsIlluminatedAtTime(State& state, const GameEntity& entity, const float astro_time) {
    const float age_duration = 75.f;
    const float astro_illumination_duration = 30.f;
    const float astro_future_sight_duration = age_duration + astro_illumination_duration / 2.f;

    bool is_responder = entity.associated_entity_record.id != -1;

    if (is_responder) {
      auto& associated_entity = *EntityManager::FindEntity(state, entity.associated_entity_record);
      float checked_time;

      if (entity.is_astro_synced) {
        checked_time = astro_time;
      } else if (entity.requires_astro_sync && !associated_entity.is_astro_synced) {
        return false;
      } else {
        checked_time = astro_time + astro_future_sight_duration;
      }

      return IsIlluminatedAtTime(state, associated_entity, checked_time);
    } else {
      return (
        entity.game_activation_time != -1.f &&
        astro_time >= entity.astro_activation_time &&
        astro_time < entity.astro_activation_time + astro_illumination_duration
      );
    }
  }

  // @todo rename LightPillar
  behavior LightPost {
    addMeshes() {
      meshes.light_post_placeholder = MODEL_MESH("./astro/3d_models/light_post/placeholder.obj", 500);
      meshes.light_post_pillar = MODEL_MESH("./astro/3d_models/light_post/pillar.obj", 500);
      meshes.light_post_lamp = MODEL_MESH("./astro/3d_models/light_post/lamp.obj", 500);
    }

    getMeshes() {
      return_meshes({
        meshes.light_post_pillar,
        meshes.light_post_lamp
      });
    }

    getPlaceholderMesh() {
      return meshes.light_post_placeholder;
    }

    timeEvolve() {
      auto& meshes = state.meshes;

      const tVec4f base_lamp_color = tVec4f(1.f, 0.8f, 0.4f, 0.f);
      const tVec4f illuminated_lamp_color = tVec4f(1.f, 0.8f, 0.5f, 0.4f);
      const float activation_delay = 0.25f;

      float time_since_casting_stun = tachyon->scene.scene_time - state.spells.stun_start_time;

      bool did_cast_stun_delayed = (
        // state.astro_turn_speed == 0.f &&
        time_since_casting_stun < activation_delay + 0.25f &&
        time_since_casting_stun > activation_delay
      );

      for_entities(state.light_posts) {
        auto& entity = state.light_posts[i];
        float player_distance = tVec3f::distance(state.player_position, entity.position);

        bool is_responder = IsResponder(entity);
        bool is_illuminated = IsIlluminatedAtTime(state, entity, state.astro_time);

        // Responders "observe" their associated light pillar in the future,
        // and become illuminated if that light pillar is in turn illuminated
        if (is_responder) {
          auto& associated_entity = *EntityManager::FindEntity(state, entity.associated_entity_record);

          if (is_illuminated && !entity.did_activate) {
            entity.did_activate = true;
            entity.game_activation_time = tachyon->scene.scene_time;
            // @todo unnecessary for responders (?)
            entity.astro_activation_time = state.astro_time;

            if (player_distance < 12000.f) {
              Sfx::PlaySound(SFX_LIGHT_POST_ACTIVATE, 1.f);
            }
          } else
          // @todo description
          if (
            is_illuminated &&
            !entity.is_astro_synced &&
            !IsResponder(associated_entity) &&
            !IsIlluminatedAtTime(state, associated_entity, state.astro_time) &&
            state.spells.did_cast_stun_this_frame &&
            tVec3f::distance(state.player_position, associated_entity.position) < 6000.f
          ) {
            entity.is_astro_synced = true;
            associated_entity.is_astro_synced = true;

            Sfx::PlaySound(SFX_LIGHT_POST_ASTRO_SYNCED, 1.f);
          } else
          // @todo description
          if (
            is_illuminated &&
            !entity.is_astro_synced &&
            entity.requires_astro_sync &&
            associated_entity.is_astro_synced &&
            IsResponder(associated_entity) &&
            state.spells.did_cast_stun_this_frame &&
            tVec3f::distance(state.player_position, GetFinalAssociatedEntity(state, entity)->position) < 6000.f
          ) {
            entity.is_astro_synced = true;

            Sfx::PlaySound(SFX_LIGHT_POST_ASTRO_SYNCED_2, 1.f);
          }
        // Regular light pillars should illuminate when the player casts stun near them
        } else {
          if (!is_illuminated && did_cast_stun_delayed && player_distance < 6000.f) {
            // Offset the time by -1.f to give a little wiggle room for
            // going back in time and still seeing the illuminated light
            float astro_activation_time_offset = -1.f;

            if (state.astro_turn_speed < 0.f) {
              // When astro turning backward, offset the time back further
              // to minimize "overshooting" if we happen to cast stun while
              // the turn is still slowing down
              astro_activation_time_offset += state.astro_turn_speed * 50.f;
            }

            entity.astro_activation_time = state.astro_time + astro_activation_time_offset;

            is_illuminated = true;
          }

          if (is_illuminated && !entity.did_activate) {
            entity.game_activation_time = tachyon->scene.scene_time;
            entity.did_activate = true;

            Sfx::PlaySound(SFX_LIGHT_POST_ACTIVATE, 1.f);
          }
        }

        if (!is_illuminated) {
          entity.did_activate = false;
        }

        // Pillar
        {
          auto& pillar = objects(meshes.light_post_pillar)[i];

          pillar.position = entity.position;
          pillar.scale = entity.scale;
          pillar.rotation = entity.orientation;

          if (is_responder) {
            pillar.color = tVec3f(0.1f);
          } else {
            pillar.color = tVec3f(0.9f, 0.8f, 0.8f);
          }

          commit(pillar);
        }

        // Lamp
        {
          auto& lamp = objects(meshes.light_post_lamp)[i];

          lamp.position = entity.position;
          lamp.scale = entity.scale;
          lamp.rotation = entity.orientation;
          lamp.material = tVec4f(1.f, 0, 0, 1.f);

          if (is_illuminated) {
            float alpha = 3.f * (tachyon->scene.scene_time - entity.game_activation_time);
            if (alpha < 0.f) alpha = 0.f;
            if (alpha > 1.f) alpha = 1.f;

            lamp.color = tVec4f::lerp(base_lamp_color, illuminated_lamp_color, alpha);
          } else {
            lamp.color = base_lamp_color;
          }

          commit(lamp);
        }

        // Light
        {
          if (entity.light_id == -1) {
            // @todo handle disposal
            entity.light_id = create_point_light();
          }

          auto& light = *get_point_light(entity.light_id);

          light.position = entity.position;
          light.position.y += entity.scale.y * 1.3f;
          light.radius = 5000.f;
          light.power = 3.f;
          light.color = tVec3f(1.f, 0.9f, 0.7f);

          if (is_illuminated) {
            float alpha = tachyon->scene.scene_time - entity.game_activation_time;
            if (alpha < 0.f) alpha = 0.f;
            if (alpha > 1.f) alpha = 1.f;

            light.radius = 5000.f * alpha;
            light.power = 3.f * alpha;
            light.glow_power = (2.f + 0.5f * sinf(tachyon->scene.scene_time)) * alpha;
          } else {
            light.radius = 0.f;
            light.power = 0.f;
          }
        }
      }
    }
  };
}