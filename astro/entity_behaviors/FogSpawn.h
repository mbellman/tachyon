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

        // @todo check for editor in dev mode only
        if (!state.is_level_editor_open && !IsDuringActiveTime(entity, state)) continue;

        tFogVolume volume;
        volume.position = entity.position;
        volume.radius = 30000.f; // @todo make configurable
        volume.thickness = entity.scale.x / 1500.f;

        if (state.is_nighttime) {
          volume.color = tVec3f(0.2f, 0.3f, 0.7f);
        } else {
          volume.color = tVec3f(0.5f, 0.5f, 0.7f);
        }

        fog_volumes.push_back(volume);
      }

      // @hack Add permanent player-aligned fog during certain portions of the game
      // @todo move this to time_evolution.cpp
      {
        auto& fx = tachyon->fx;
        float thickness = 1.f - Tachyon_InverseLerp(astro_time_periods.distant_past, astro_time_periods.past, state.astro_time);

        tFogVolume volume;
        volume.position = state.player_position;
        volume.radius = 500000.f;
        volume.color = tVec3f(0.7f, 0.85f, 1.f);
        volume.thickness = thickness;

        float time_since_spawning = time_since(state.last_spawn_time);

        if (time_since_spawning < 2.5f) {
          float alpha = time_since_spawning / 2.5f;

          volume.color = tVec3f(0.5f, 0.6f, 0.7f);
          volume.thickness = Tachyon_Lerpf(1.f, thickness, alpha * alpha * alpha);

          fx.fog_visibility = Tachyon_Lerpf(4000.f, 15000.f, alpha);
        } else {
          fx.fog_visibility = Tachyon_Lerpf(fx.fog_visibility, 25000.f, thickness);
        }

        fog_volumes.push_back(volume);
      }
    }
  };
}