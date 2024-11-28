#include "cosmodrone/mesh_library.h"

using namespace Cosmodrone;

static std::vector<MeshAsset> placeable_mesh_assets;
static std::vector<MeshAsset> generated_mesh_assets;

// @bug once in a while, meshes don't load in at the beginning! figure this out
static void LoadShipPartMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  // @todo define in a list
  auto hull_mesh = Tachyon_LoadMesh("./cosmodrone/assets/test-ship/hull.obj");
  auto streams_mesh = Tachyon_LoadMesh("./cosmodrone/assets/test-ship/streams.obj");
  auto thrusters_mesh = Tachyon_LoadMesh("./cosmodrone/assets/test-ship/thrusters.obj");
  auto trim_mesh = Tachyon_LoadMesh("./cosmodrone/assets/test-ship/trim.obj");

  meshes.hull = Tachyon_AddMesh(tachyon, hull_mesh, 1);
  meshes.streams = Tachyon_AddMesh(tachyon, streams_mesh, 1);
  meshes.thrusters = Tachyon_AddMesh(tachyon, thrusters_mesh, 1);
  meshes.trim = Tachyon_AddMesh(tachyon, trim_mesh, 1);
}

static void LoadPlaceableMeshes(Tachyon* tachyon, State& state) {
  #define load_mesh(__name) meshes.__name = Tachyon_AddMesh(tachyon, Tachyon_LoadMesh("./cosmodrone/assets/station-parts/" #__name ".obj"), 5000)

  auto& meshes = state.meshes;

  meshes.zone_target = Tachyon_AddMesh(tachyon, Tachyon_CreateSphereMesh(16), 100);

  load_mesh(antenna_1);
  load_mesh(antenna_2);
  load_mesh(antenna_3);
  load_mesh(radio_tower_1);
  load_mesh(module_1);
  load_mesh(module_2);
  load_mesh(habitation_1);
  load_mesh(habitation_2);
  load_mesh(silo_2);
  load_mesh(silo_3);
  load_mesh(silo_4);
  load_mesh(silo_5);
  load_mesh(silo_6);
  load_mesh(torus_1);
  load_mesh(station_torus_1);
  load_mesh(station_torus_2);
  load_mesh(station_torus_3);
  load_mesh(station_base);
  load_mesh(spire_fortress);
  load_mesh(gate_tower_1);
  load_mesh(solar_panel_1);
  load_mesh(solar_panel_2);
  load_mesh(girder_1);
  load_mesh(girder_2);
  load_mesh(girder_3);
  load_mesh(girder_4);
  load_mesh(girder_5);
  load_mesh(girder_6);
  load_mesh(grate_1);
  load_mesh(grate_2);
  load_mesh(track_1);
  load_mesh(light_1);
  load_mesh(light_2);
  load_mesh(light_3);
  load_mesh(gas_flare_1_spawn);

  // @todo refactor
  placeable_mesh_assets.push_back({
    .mesh_name = "zone_target",
    .mesh_index = meshes.zone_target,
    .placeholder = true,
    .defaults = {
      .scale = tVec3f(3000.f),
      .color = tVec4f(1.f, 1.f, 0.1f, 0.2f),
      .material = tVec4f(0.6f, 0, 0, 0)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "antenna_1",
    .mesh_index = meshes.antenna_1,
    .defaults = {
      .scale = tVec3f(2000.f),
      .color = tVec3f(1.f),
      .material = tVec4f(0.4f, 1.f, 0, 0)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "antenna_2",
    .mesh_index = meshes.antenna_2,
    .placeholder = true,
    .defaults = {
      .scale = tVec3f(8000.f),
      .color = tVec3f(1.f),
      .material = tVec4f(1.f, 1.f, 0, 0)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "antenna_3",
    .mesh_index = meshes.antenna_3,
    .defaults = {
      .scale = tVec3f(4000.f),
      .color = tVec4f(1.f, 1.f, 1.f, 0.2f),
      .material = tVec4f(0.6f, 0, 0.1f, 0.6f)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "radio_tower_1",
    .mesh_index = meshes.radio_tower_1,
    .defaults {
      .scale = tVec3f(5000.f),
      .color = tVec3f(1.f),
      .material = tVec4f(0.9f, 0, 0, 0)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "module_1",
    .mesh_index = meshes.module_1,
    .defaults = {
      .scale = tVec3f(1000.f),
      .color = tVec3f(1.f, 0.7f, 0.2f),
      .material = tVec4f(0.6f, 1.f, 0, 0)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "module_2",
    .mesh_index = meshes.module_2,
    .placeholder = true,
    .defaults = {
      .scale = tVec3f(8000.f),
      .color = tVec3f(0.4f),
      .material = tVec4f(0.9f, 0, 0, 0.3f)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "habitation_1",
    .mesh_index = meshes.habitation_1,
    .defaults = {
      .scale = tVec3f(6000.f),
      .color = tVec3f(1.f),
      .material = tVec4f(1.f, 0, 0, 0.1f)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "habitation_2",
    .mesh_index = meshes.habitation_2,
    .defaults = {
      .scale = tVec3f(6000.f),
      .color = tVec3f(1.f),
      .material = tVec4f(0.4f, 1.f, 0, 0)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "silo_2",
    .mesh_index = meshes.silo_2,
    .defaults = {
      .scale = tVec3f(2000.f),
      .color = tVec3f(1.f, 0.7f, 0.6f),
      .material = tVec4f(0.4f, 1.f, 0, 0)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "silo_3",
    .mesh_index = meshes.silo_3,
    .placeholder = true,
    .defaults = {
      .scale = tVec3f(3000.f),
      .color = tVec3f(1.f),
      .material = tVec4f(1.f, 0, 0.1f, 0.3f)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "silo_4",
    .mesh_index = meshes.silo_4,
    .defaults = {
      .scale = tVec3f(6000.f),
      .color = tVec3f(1.f),
      .material = tVec4f(0.8f, 1.f, 0, 0.1f)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "silo_5",
    .mesh_index = meshes.silo_5,
    .defaults = {
      .scale = tVec3f(9000.f),
      .color = tVec3f(0.8f, 0.4f, 0.3f),
      .material = tVec4f(0.4f, 1.f, 0, 0.1f)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "silo_6",
    .mesh_index = meshes.silo_6,
    .placeholder = true,
    .defaults = {
      .scale = tVec3f(40000.f),
      .color = tVec3f(0.8f, 0.7f, 0.6f),
      .material = tVec4f(0.5f, 1.f, 0, 0)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "torus_1",
    .mesh_index = meshes.torus_1
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "station_torus_1",
    .mesh_index = meshes.station_torus_1,
    .defaults = {
      .scale = tVec3f(100000.f),
      .color = tVec3f(1.f),
      .material = tVec4f(1.f, 0, 0.1f, 0.3f)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "station_torus_2",
    .mesh_index = meshes.station_torus_2,
    .placeholder = true,
    .defaults = {
      .scale = tVec3f(100000.f),
      .color = tVec3f(1.f),
      .material = tVec4f(1.f, 0, 0.1f, 0.3f)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "station_torus_3",
    .mesh_index = meshes.station_torus_3,
    .placeholder = true,
    .defaults = {
      .scale = tVec3f(30000.f),
      .color = tVec3f(1.f),
      .material = tVec4f(1.f, 0, 0.1f, 0.2f)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "station_base",
    .mesh_index = meshes.station_base,
    .defaults = {
      .scale = tVec3f(80000.f),
      .color = tVec3f(1.f),
      .material = tVec4f(1.f, 0, 0, 0.2f)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "spire_fortress",
    .mesh_index = meshes.spire_fortress,
    .defaults = {
      .scale = tVec3f(400000.f),
      .color = tVec3f(1.f),
      .material = tVec4f(0.3f, 1.f, 0, 0)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "gate_tower_1",
    .mesh_index = meshes.gate_tower_1,
    .defaults = {
      .scale = tVec3f(1000000.f),
      .color = tVec3f(0.5f),
      .material = tVec4f(1.f, 0, 0, 0.4f)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "solar_panel_1",
    .mesh_index = meshes.solar_panel_1
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "solar_panel_2",
    .mesh_index = meshes.solar_panel_2,
    .placeholder = true,
    .defaults = {
      .scale = tVec3f(5000.f),
      .color = 0x44F1,
      .material = tVec4f(0.2f, 1.f, 0.3f, 1.f)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "girder_1",
    .mesh_index = meshes.girder_1,
    .defaults = {
      .scale = tVec3f(2000.f),
      .color = tVec3f(1.f),
      .material = tVec4f(0.4f, 1.f, 0, 0)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "girder_2",
    .mesh_index = meshes.girder_2,
    .defaults = {
      .scale = tVec3f(4000.f),
      .color = tVec3f(1.f),
      .material = tVec4f(0.5f, 1.f, 0, 0)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "girder_3",
    .mesh_index = meshes.girder_3,
    .defaults = {
      .scale = tVec3f(6000.f),
      .color = tVec3f(1.f),
      .material = tVec4f(0.6f, 1.f, 0, 0)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "girder_4",
    .mesh_index = meshes.girder_4,
    .defaults = {
      .scale = tVec3f(8000.f),
      .color = tVec3f(1.f),
      .material = tVec4f(0.2f, 1.f, 0, 0)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "girder_5",
    .mesh_index = meshes.girder_5,
    .defaults = {
      .scale = tVec3f(8000.f),
      .color = tVec3f(1.f),
      .material = tVec4f(0.4f, 1.f, 0, 0)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "girder_6",
    .mesh_index = meshes.girder_6,
    .placeholder = true,
    .defaults = {
      .scale = tVec3f(20000.f),
      .color = tVec3f(0.6, 0.1f, 0.1f),
      .material = tVec4f(0.5f, 1.f, 0, 0)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "grate_1",
    .mesh_index = meshes.grate_1,
    .defaults = {
      .scale = tVec3f(3000.f),
      .color = tVec3f(1.f, 0.2f, 0.1f),
      .material = tVec4f(0.5f, 0, 0, 0.6f)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "grate_2",
    .mesh_index = meshes.grate_2,
    .defaults = {
      .scale = tVec3f(6000.f),
      .color = tVec3f(1.f, 0.2f, 0.1f),
      .material = tVec4f(0.5f, 0, 0, 0.6f)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "track_1",
    .mesh_index = meshes.track_1,
    .defaults = {
      .scale = tVec3f(12000.f),
      .color = tVec3f(1.f),
      .material = tVec4f(0.2f, 0, 0.2f, 0.2f)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "light_1",
    .mesh_index = meshes.light_1,
    .placeholder = true,
    .defaults = {
      .scale = tVec3f(1000.f),
      .color = tVec4f(1.f, 0.2f, 0.1f, 1.f),
      .material = tVec4f(1.f, 0, 1.f, 1.f)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "light_2",
    .mesh_index = meshes.light_2,
    .placeholder = true,
    .defaults = {
      .scale = tVec3f(1000.f),
      .color = tVec4f(0.1f, 0.4f, 1.f, 1.f),
      .material = tVec4f(1.f, 0, 1.f, 1.f)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "light_3",
    .mesh_index = meshes.light_3,
    .placeholder = true,
    .defaults = {
      .scale = tVec3f(1000.f),
      .color = tVec4f(1.f, 0.9f, 0.8f, 1.f),
      .material = tVec4f(1.f, 0, 1.f, 1.f)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "gas_flare_1_spawn",
    .mesh_index = meshes.gas_flare_1_spawn,
    .placeholder = true,
    .defaults = {
      .scale = tVec3f(6000.f),
      .color = tVec4f(1.f, 0, 0, 0.5f)
    }
  });
}

static void LoadGeneratedMeshes(Tachyon* tachyon, State& state) {
  #define load_mesh(__name) meshes.__name = Tachyon_AddMesh(tachyon, Tachyon_LoadMesh("./cosmodrone/assets/station-parts/generated/" #__name ".obj"), 5000)

  auto& meshes = state.meshes;

  meshes.gas_flare_1 = Tachyon_AddMesh(tachyon, Tachyon_CreateSphereMesh(16), 5000);

  load_mesh(antenna_2_frame);
  load_mesh(antenna_2_receivers);

  load_mesh(silo_3_body);
  load_mesh(silo_3_frame);

  load_mesh(silo_6_body);
  load_mesh(silo_6_frame);

  load_mesh(girder_6_core);
  load_mesh(girder_6_frame);

  load_mesh(habitation_1_windows);

  load_mesh(module_2_core);
  load_mesh(module_2_frame);

  load_mesh(solar_panel_2_cells);
  load_mesh(solar_panel_2_frame);

  load_mesh(station_torus_2_body);
  load_mesh(station_torus_2_supports);
  load_mesh(station_torus_2_frame);

  load_mesh(station_torus_3_body);
  load_mesh(station_torus_3_frame);

  load_mesh(light_1_base);
  load_mesh(light_1_bulb);

  load_mesh(light_2_base);
  load_mesh(light_2_bulb);

  load_mesh(light_3_base);
  load_mesh(light_3_bulb);

  generated_mesh_assets = {
    // antenna_2
    {
      .mesh_index = meshes.antenna_2_frame,
      .generated_from = meshes.antenna_2,
      .defaults = {
        .material = tVec4f(1.f, 1.f, 0, 0)
      }
    },
    {
      .mesh_index = meshes.antenna_2_receivers,
      .generated_from = meshes.antenna_2,
      .defaults = {
        .material = tVec4f(0.9f, 0, 0, 0.2f)
      }
    },

    // silo_3
    {
      .mesh_index = meshes.silo_3_body,
      .generated_from = meshes.silo_3,
      .defaults = {
        .material = tVec4f(1.f, 0, 0.2f, 0.3f)
      }
    },
    {
      .mesh_index = meshes.silo_3_frame,
      .generated_from = meshes.silo_3,
      .defaults = {
        .color = tVec3f(0.2f),
        .material = tVec4f(0.2f, 0, 0, 0.3f)
      }
    },

    // silo_6,
    {
      .mesh_index = meshes.silo_6_body,
      .generated_from = meshes.silo_6,
      .defaults = {
        .color = tVec3f(0.8f, 0.7f, 0.6f),
        .material = tVec4f(0.6f, 1.f, 0, 0)
      }
    },
    {
      .mesh_index = meshes.silo_6_frame,
      .generated_from = meshes.silo_6,
      .defaults = {
        .color = tVec3f(0.5f),
        .material = tVec4f(0.2f, 1.f, 0, 0)
      }
    },

    // girder_6
    {
      .mesh_index = meshes.girder_6_core,
      .generated_from = meshes.girder_6,
      .defaults = {
        .color = tVec3f(0.6, 0.1f, 0.1f),
        .material = tVec4f(0.5f, 1.f, 0, 0)
      }
    },
    {
      .mesh_index = meshes.girder_6_frame,
      .generated_from = meshes.girder_6,
      .defaults = {
        .material = tVec4f(0.4f, 1.f, 0, 0)
      }
    },

    // habitation_1,
    {
      .mesh_index = meshes.habitation_1_windows,
      .generated_from = meshes.habitation_1,
      .defaults {
        .color = tVec4f(0.2f, 0.2f, 0.2f, 1.f),
        .material = tVec4f(0.3f, 0, 1.f, 0)
      }
    },

    // module_2
    {
      .mesh_index = meshes.module_2_core,
      .generated_from = meshes.module_2
    },
    {
      .mesh_index = meshes.module_2_frame,
      .generated_from = meshes.module_2,
      .defaults = {
        .material = tVec4f(0.4f, 1.f, 0, 0)
      }
    },

    // solar_panel_2
    {
      .mesh_index = meshes.solar_panel_2_cells,
      .generated_from = meshes.solar_panel_2,
      .defaults = {
        .color = 0x44F1,
        .material = tVec4f(0.2f, 1.f, 0.3f, 1.f) 
      }
    },
    {
      .mesh_index = meshes.solar_panel_2_frame,
      .generated_from = meshes.solar_panel_2,
      .defaults = {
        .material = tVec4f(0.3f, 1.f, 0, 0)
      }
    },

    // station_torus_2
    {
      .mesh_index = meshes.station_torus_2_body,
      .generated_from = meshes.station_torus_2
    },
    {
      .mesh_index = meshes.station_torus_2_supports,
      .generated_from = meshes.station_torus_2,
      .defaults = {
        .material = tVec4f(0.4f, 1.f, 0, 0)
      }
    },
    {
      .mesh_index = meshes.station_torus_2_frame,
      .generated_from = meshes.station_torus_2,
      .defaults = {
        .color = tVec3f(1.f, 0.2f, 0.2f),
        .material = tVec4f(0.4f, 1.f, 0, 0)
      }
    },

    // station_torus_3
    {
      .mesh_index = meshes.station_torus_3_body,
      .generated_from = meshes.station_torus_3,
      .defaults = {
        .material = tVec4f(1.f, 0, 0, 0.2f)
      }
    },
    {
      .mesh_index = meshes.station_torus_3_frame,
      .generated_from = meshes.station_torus_3,
      .defaults = {
        .material = tVec4f(0.1f, 1.f, 0, 0)
      }
    },

    // light_1
    {
      .mesh_index = meshes.light_1_base,
      .generated_from = meshes.light_1,
      .defaults = {
        .color = tVec3f(1.f),
        .material = tVec4f(0.7f, 1.f, 0, 0)
      }
    },
    {
      .mesh_index = meshes.light_1_bulb,
      .generated_from = meshes.light_1,
      .defaults = {
        .color = tVec4f(1.f, 0.2f, 0.1f, 1.f),
        .material = tVec4f(1.f, 0, 0, 0)
      }
    },

    // light_2
    {
      .mesh_index = meshes.light_2_base,
      .generated_from = meshes.light_2,
      .defaults = {
        .color = tVec3f(1.f),
        .material = tVec4f(0.7f, 1.f, 0, 0)
      }
    },
    {
      .mesh_index = meshes.light_2_bulb,
      .generated_from = meshes.light_2,
      .defaults = {
        .color = tVec4f(0.1f, 0.4f, 1.f, 1.f),
        .material = tVec4f(1.f, 0, 0, 0)
      }
    },

    // light_3
    {
      .mesh_index = meshes.light_3_base,
      .generated_from = meshes.light_3,
      .defaults = {
        .color = tVec3f(1.f),
        .material = tVec4f(0.7f, 1.f, 0, 0)
      }
    },
    {
      .mesh_index = meshes.light_3_bulb,
      .generated_from = meshes.light_3,
      .defaults = {
        .color = tVec4f(1.f, 0.8f, 0.6f, 1.f),
        .material = tVec4f(1.f, 0, 0, 0)
      }
    },

    // gas_flare_1,
    {
      .mesh_index = meshes.gas_flare_1,
      .generated_from = meshes.gas_flare_1_spawn,
      .defaults = {
        .scale = tVec3f(6000.f, 12000.f, 6000.f),
        .color = tVec4f(1.f, 0.2f, 0.1f, 0.8f)
      }
    }
  };
}

static void LoadBackgroundMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  // @todo have separate meshes for each celestial body
  // @todo define a list of celestial bodies + properties
  auto planet_mesh = Tachyon_CreateSphereMesh(40);

  meshes.planet = Tachyon_AddMesh(tachyon, planet_mesh, 2);
  meshes.earth_atmosphere = Tachyon_AddMesh(tachyon, planet_mesh, 1);
  meshes.space_elevator = Tachyon_AddMesh(tachyon, Tachyon_LoadMesh("./cosmodrone/assets/space-elevator.obj"), 1);

  mesh(meshes.earth_atmosphere).type = VOLUMETRIC_MESH;
}

static void LoadEntityMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  // Elevator cars
  // @todo make this a placeable mesh
  {
    auto elevator_car = Tachyon_LoadMesh("./cosmodrone/assets/elevator_car_1.obj");
    auto elevator_car_frame = Tachyon_LoadMesh("./cosmodrone/assets/elevator_car_1_frame.obj");

    meshes.elevator_car_1 = Tachyon_AddMesh(tachyon, elevator_car, 1);
    meshes.elevator_car_1_frame = Tachyon_AddMesh(tachyon, elevator_car_frame, 1);
  }

  // HUD
  {
    auto flight_arrow = Tachyon_LoadMesh("./cosmodrone/assets/flight_arrow.obj");
    auto wedge = Tachyon_LoadMesh("./cosmodrone/assets/hud_wedge.obj");

    meshes.hud_flight_arrow = Tachyon_AddMesh(tachyon, flight_arrow, 16);
    meshes.hud_wedge = Tachyon_AddMesh(tachyon, wedge, 2);
  }
}

static void LoadDebugMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  auto cube_mesh = Tachyon_CreateCubeMesh();

  meshes.cube = Tachyon_AddMesh(tachyon, cube_mesh, 6);
  meshes.editor_guideline = Tachyon_AddMesh(tachyon, cube_mesh, 3000);

  // @todo description
  {
    auto position_mesh = Tachyon_LoadMesh("./cosmodrone/assets/editor/position-action-indicator.obj");
    auto rotation_mesh = Tachyon_LoadMesh("./cosmodrone/assets/editor/rotate-action-indicator.obj");
    auto scale_mesh = Tachyon_LoadMesh("./cosmodrone/assets/editor/scale-action-indicator.obj");

    meshes.editor_position = add_mesh(position_mesh, 1);
    meshes.editor_rotation = add_mesh(rotation_mesh, 1);
    meshes.editor_scale = add_mesh(scale_mesh, 1);
  }
}

void MeshLibrary::LoadMeshes(Tachyon* tachyon, State& state) {
  // @todo dev mode only
  auto start_time = Tachyon_GetMicroseconds();

  LoadShipPartMeshes(tachyon, state);
  LoadPlaceableMeshes(tachyon, state);
  LoadGeneratedMeshes(tachyon, state);
  LoadBackgroundMeshes(tachyon, state);
  LoadEntityMeshes(tachyon, state);
  LoadDebugMeshes(tachyon, state);

  // @todo dev mode only
  {
    auto load_time = (Tachyon_GetMicroseconds() - start_time) / 1000;

    add_console_message("Loaded meshes in " + std::to_string(load_time) + "ms", tVec3f(1.f));
  }

  // Define highest cascades per mesh
  {
    auto& records = tachyon->mesh_pack.mesh_records;
    auto& meshes = state.meshes;

    mesh(meshes.girder_1).shadow_cascade_ceiling = 3;
    mesh(meshes.grate_1).shadow_cascade_ceiling = 2;
    mesh(meshes.hud_flight_arrow).shadow_cascade_ceiling = 0;
    mesh(meshes.hud_wedge).shadow_cascade_ceiling = 0;

    // @todo dev mode only
    mesh(meshes.editor_guideline).shadow_cascade_ceiling = 0;
    mesh(meshes.cube).shadow_cascade_ceiling = 0;
  }

  Tachyon_InitializeObjects(tachyon);
}

const std::vector<MeshAsset>& MeshLibrary::GetPlaceableMeshAssets() {
  return placeable_mesh_assets;
}


const std::vector<MeshAsset>& MeshLibrary::GetGeneratedMeshAssets() {
  return generated_mesh_assets;
}