#pragma once

#include "astro/entity_behaviors/behavior.h"
#include "astro/astrolabe.h"

namespace astro {
  behavior FogSpawn {
    addMeshes() {
      meshes.fog_spawn_placeholder = SPHERE_MESH(500);

      mesh(meshes.fog_spawn_placeholder).shadow_cascade_ceiling = 0;
    }

    getMeshes() {
      return_meshes({});
    }

    getPlaceholderMesh() {
      return meshes.fog_spawn_placeholder;
    }

    timeEvolve() {
      auto& fog_volumes = tachyon->fog_volumes;

      fog_volumes.clear();

      // @todo during gameplay, only add fog volumes within player view
      for_entities(state.fog_spawns) {
        auto& entity = state.fog_spawns[i];

        // Culling (only when the level editor is not open,
        // as we want to be able to see all fog spawn volumes
        // in the editor)
        // @todo check for editor in dev mode only
        if (!state.is_level_editor_open) {
          if (!IsDuringActiveTime(entity, state)) continue;

          if (
            entity.unique_name.starts_with("tutorial") &&
            state.current_location != Location::TUTORIAL
          ) {
            continue;
          }
        }

        tFogVolume volume;
        volume.position = entity.position;
        volume.radius = 30000.f; // @todo make configurable
        volume.thickness = entity.scale.x / 1500.f;

        if (!state.is_level_editor_open) {
          // Allow fog to "fade in" from its astro start time,
          // as well as "fade out" when we travel back prior
          // to its start time
          float astro_time_thickness_factor = Tachyon_InverseLerp(
            entity.astro_start_time,
            entity.astro_start_time + 30.f,
            state.astro_time
          );

          volume.thickness *= astro_time_thickness_factor;
        }

        if (state.is_nighttime) {
          volume.color = tVec3f(0.2f, 0.3f, 0.7f);
        } else {
          volume.color = tVec3f(0.5f, 0.5f, 0.7f);
        }

        fog_volumes.push_back(volume);
      }

      // Spawn fog
      // @todo move this elsewhere
      {
        float time_since_spawning = time_since(state.last_spawn_time);

        if (time_since_spawning < 4.f) {
          float alpha = Tachyon_EaseOutSine(time_since_spawning / 4.f);

          tFogVolume volume;
          volume.position = state.player_position;
          volume.radius = 500000.f;
          volume.color = tVec3f(0.4f, 0.5f, 0.6f);
          volume.thickness = Tachyon_Lerpf(1.f, 0.f, alpha);

          fog_volumes.push_back(volume);
        }
      }
    }
  };
}