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

        tFogVolume volume;
        volume.position = entity.position;
        volume.radius = 30000.f;
        volume.thickness = 1.f;

        if (state.is_nighttime) {
          volume.color = tVec3f(0.2f, 0.3f, 0.7f);
        } else {
          volume.color = tVec3f(0.5f, 0.5f, 0.7f);
        }

        fog_volumes.push_back(volume);
      }

      // @hack Add permanent player-aligned fog during certain portions of the game
      {
        float thickness = 1.f - Tachyon_InverseLerp(astro_time_periods.distant_past, astro_time_periods.past, state.astro_time);

        tFogVolume volume;
        volume.position = state.player_position;
        volume.radius = 500000.f;
        volume.color = tVec3f(0.7f, 0.85f, 1.f);
        volume.thickness = thickness;

        tachyon->fx.fog_visibility = Tachyon_Lerpf(tachyon->fx.fog_visibility, 25000.f, thickness);

        fog_volumes.push_back(volume);
      }
    }
  };
}