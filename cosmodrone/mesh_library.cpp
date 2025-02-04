#include "cosmodrone/background_vehicles.h"
#include "cosmodrone/mesh_library.h"
#include "cosmodrone/procedural_generation.h"

using namespace Cosmodrone;

static std::vector<MeshAsset> placeable_mesh_assets;
static std::vector<MeshAsset> generated_mesh_assets;

static void LoadShipPartMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  // @todo define in a list
  auto hull_mesh = Tachyon_LoadMesh("./cosmodrone/assets/drone/hull.obj");
  auto streams_mesh = Tachyon_LoadMesh("./cosmodrone/assets/drone/streams.obj");
  auto thrusters_mesh = Tachyon_LoadMesh("./cosmodrone/assets/drone/thrusters.obj");
  auto trim_mesh = Tachyon_LoadMesh("./cosmodrone/assets/drone/trim.obj");
  auto jets_mesh = Tachyon_LoadMesh("./cosmodrone/assets/drone/jets.obj");

  meshes.hull = Tachyon_AddMesh(tachyon, hull_mesh, 1);
  meshes.streams = Tachyon_AddMesh(tachyon, streams_mesh, 1);
  meshes.thrusters = Tachyon_AddMesh(tachyon, thrusters_mesh, 1);
  meshes.trim = Tachyon_AddMesh(tachyon, trim_mesh, 1);
  meshes.jets = Tachyon_AddMesh(tachyon, jets_mesh, 1);

  // @todo use GLOW_MESH once the shader is ready
  mesh(meshes.jets).type = tMeshType::WIREFRAME_MESH;
}

static void LoadPlaceableMeshes(Tachyon* tachyon, State& state) {
  #define load_mesh(__name) meshes.__name = Tachyon_AddMesh(tachyon, Tachyon_LoadMesh("./cosmodrone/assets/station-parts/" #__name ".obj"), 3000)

  #define load_mesh_with_2_lods(__name) meshes.__name =\
    Tachyon_AddMesh(\
      tachyon,\
      Tachyon_LoadMesh("./cosmodrone/assets/station-parts/" #__name ".obj"),\
      Tachyon_LoadMesh("./cosmodrone/assets/station-parts/" #__name "_lod_2.obj"),\
      3000\
    )

  auto& meshes = state.meshes;

  meshes.zone_target = Tachyon_AddMesh(tachyon, Tachyon_CreateSphereMesh(16), 100);
  meshes.vehicle_target = Tachyon_AddMesh(tachyon, Tachyon_CreateSphereMesh(16), 100);

  load_mesh_with_2_lods(antenna_1);
  load_mesh(antenna_2);
  load_mesh(antenna_3);
  load_mesh(antenna_4);
  load_mesh(machine_1);
  load_mesh(machine_2);
  load_mesh(machine_3);
  load_mesh(radio_tower_1);
  load_mesh(module_1);
  load_mesh(module_2);
  load_mesh(habitation_1);
  load_mesh(habitation_2);
  load_mesh(habitation_3);
  load_mesh(habitation_4);
  load_mesh_with_2_lods(silo_2);
  load_mesh(silo_3);
  load_mesh(silo_4);
  load_mesh(silo_5);
  load_mesh(silo_6);
  load_mesh(silo_7);
  load_mesh(torus_1);
  load_mesh(elevator_torus_1);
  load_mesh(station_torus_1);
  load_mesh(station_torus_2);
  load_mesh(station_torus_3);
  load_mesh(station_torus_4);
  load_mesh(station_platform_1);
  load_mesh(platform);
  load_mesh(base_1);
  load_mesh(building_1);
  load_mesh(station_base);
  load_mesh(gate_tower_1);
  load_mesh(solar_panel_1);
  load_mesh(solar_panel_2);
  load_mesh_with_2_lods(girder_1);
  load_mesh(girder_1b);
  load_mesh_with_2_lods(girder_2);
  load_mesh_with_2_lods(girder_3);
  load_mesh(girder_4);
  load_mesh_with_2_lods(girder_5);
  load_mesh(girder_6);
  load_mesh(beam_1);
  load_mesh(beam_2);
  load_mesh_with_2_lods(mega_girder_1);
  load_mesh(mega_girder_2);
  load_mesh_with_2_lods(grate_1);
  load_mesh_with_2_lods(grate_2);
  load_mesh(grate_3);
  load_mesh(track_1);
  load_mesh(light_1);
  load_mesh(light_2);
  load_mesh(light_3);
  load_mesh(light_4);
  load_mesh(charge_pad);
  load_mesh(fighter_spawn);
  load_mesh(floater_1);
  load_mesh(gas_flare_1_spawn);
  load_mesh(arch_1);
  load_mesh(station_drone);
  load_mesh(background_ship_1);

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
    .mesh_name = "vehicle_target",
    .mesh_index = meshes.vehicle_target,
    .placeholder = true,
    .defaults = {
      .scale = tVec3f(3000.f),
      .color = tVec4f(0.4f, 1.f, 0.1f, 0.2f),
      .material = tVec4f(1.f, 0, 0, 0)
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
      .color = tVec3f(1.f),
      .material = tVec4f(0.6f, 0, 0.1f, 0.4f)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "antenna_4",
    .mesh_index = meshes.antenna_4,
    .placeholder = true,
    .defaults = {
      .scale = tVec3f(2000.f),
      .color = tVec3f(1.f),
      .material = tVec4f(0.6f, 0, 0.1f, 0.2f)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "machine_1",
    .mesh_index = meshes.machine_1,
    .defaults = {
      .scale = tVec3f(3000.f),
      .material = tVec4f(1.f, 0, 0, 0.2f)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "machine_2",
    .mesh_index = meshes.machine_2,
    .defaults = {
      .scale = tVec3f(3000.f),
      .material = tVec4f(1.f, 0, 0, 0.2f)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "machine_3",
    .mesh_index = meshes.machine_3,
    .defaults = {
      .scale = tVec3f(3000.f),
      .material = tVec4f(1.f, 0, 0, 0.2f)
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
    .placeholder = true,
    .defaults = {
      .scale = tVec3f(6000.f),
      .color = tVec3f(1.f),
      .material = tVec4f(1.f, 0, 0, 0.1f)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "habitation_2",
    .mesh_index = meshes.habitation_2,
    .placeholder = true,
    .defaults = {
      .scale = tVec3f(6000.f),
      .color = tVec3f(1.f),
      .material = tVec4f(0.4f, 1.f, 0, 0)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "habitation_3",
    .mesh_index = meshes.habitation_3,
    .placeholder = true,
    .defaults = {
      .scale = tVec3f(7000.f),
      .color = tVec3f(1.f),
      .material = tVec4f(0.6f, 0, 0, 0.6f)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "habitation_4",
    .mesh_index = meshes.habitation_4,
    .placeholder = true,
    .defaults = {
      .scale = tVec3f(6000.f),
      .color = tVec3f(1.f),
      .material = tVec4f(0.6f, 0, 0, 0.4f)
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
      .scale = tVec3f(30000.f),
      .color = tVec3f(0.8f, 0.7f, 0.6f),
      .material = tVec4f(0.5f, 1.f, 0, 0)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "silo_7",
    .mesh_index = meshes.silo_7,
    .placeholder = true,
    .defaults = {
      .scale = tVec3f(7000.f)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "torus_1",
    .mesh_index = meshes.torus_1
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "elevator_torus_1",
    .mesh_index = meshes.elevator_torus_1,
    .defaults = {
      .scale = tVec3f(25000.f),
      .color = tVec3f(1.f),
      .material = tVec4f(0.6f, 0, 0, 0.3f)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "station_torus_1",
    .mesh_index = meshes.station_torus_1,
    .defaults = {
      .scale = tVec3f(150000.f),
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
      .scale = tVec3f(35000.f),
      .color = tVec3f(1.f),
      .material = tVec4f(1.f, 0, 0.1f, 0.2f)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "station_torus_4",
    .mesh_index = meshes.station_torus_4,
    .placeholder = true,
    .defaults = {
      .scale = tVec3f(100000.f),
      .color = tVec3f(1.f),
      .material = tVec4f(1.f, 0, 0.1f, 0.2f)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "station_platform_1",
    .mesh_index = meshes.station_platform_1,
    .defaults = {
      .scale = tVec3f(100000.f),
      .color = tVec3f(1.f),
      .material = tVec4f(1.f, 0, 0.1f, 0.2f)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "platform",
    .mesh_index = meshes.platform,
    .placeholder = true,
    .defaults = {
      .scale = tVec3f(75000.f),
      .material = tVec4f(0.9f, 0, 0, 0.2f)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "base_1",
    .mesh_index = meshes.base_1,
    .defaults = {
      .scale = tVec3f(40000.f),
      .material = tVec4f(0.7f, 1.f, 0, 0.2f)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "building_1",
    .mesh_index = meshes.building_1,
    .defaults = {
      .scale = tVec3f(8000.f),
      .material = tVec4f(0.7f, 0, 0, 0.1f)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "station_base",
    .mesh_index = meshes.station_base,
    .defaults = {
      .scale = tVec3f(80000.f),
      .material = tVec4f(1.f, 0, 0, 0.2f)
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
      .material = tVec4f(0.4f, 1.f, 0, 0)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "girder_1b",
    .mesh_index = meshes.girder_1b,
    .defaults = {
      .scale = tVec3f(2000.f),
      .material = tVec4f(1.f, 1.f, 0, 0)
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
    .placeholder = true,
    .defaults = {
      .scale = tVec3f(8000.f),
      .color = tVec3f(1.f),
      .material = tVec4f(0.5f, 1.f, 0, 0.5f)
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
    .mesh_name = "beam_1",
    .mesh_index = meshes.beam_1,
    .defaults = {
      .scale = tVec3f(5000.f),
      .material = tVec4f(0.8f, 0, 0, 0.1f)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "beam_2",
    .mesh_index = meshes.beam_2,
    .placeholder = true,
    .defaults = {
      .scale = tVec3f(10000.f),
      .color = tVec3f(1.f, 0.5f, 0.4f),
      .material = tVec4f(0.8f, 0, 0, 0.1f)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "mega_girder_1",
    .mesh_index = meshes.mega_girder_1,
    .defaults = {
      .scale = tVec3f(50000.f),
      .color = tVec3f(1.f),
      .material = tVec4f(0.5f, 1.f, 0, 0)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "mega_girder_2",
    .mesh_index = meshes.mega_girder_2,
    .defaults = {
      .scale = tVec3f(15000.f),
      .color = tVec3f(1.f, 0.4f, 0.1f),
      .material = tVec4f(0.9f, 0, 0, 0.4f)
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
    .mesh_name = "grate_3",
    .mesh_index = meshes.grate_3,
    .defaults = {
      .scale = tVec3f(4000.f),
      .material = tVec4f(0.9f, 1.f, 0, 0)
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
    .mesh_name = "light_4",
    .mesh_index = meshes.light_4,
    .placeholder = true,
    .defaults = {
      .scale = tVec3f(2000.f),
      .color = tVec4f(1.f, 0.9f, 0.8f, 1.f),
      .material = tVec4f(1.f, 0, 0, 1.f)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "charge_pad",
    .mesh_index = meshes.charge_pad,
    .defaults = {
      .scale = tVec3f(2000.f),
      .material = tVec4f(0.6f, 0.5f, 0, 0.1f)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "fighter_spawn",
    .mesh_index = meshes.fighter_spawn,
    .placeholder = true,
    .defaults = {
      .scale = tVec3f(5000.f)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "floater_1",
    .mesh_index = meshes.floater_1,
    .placeholder = true,
    .defaults = {
      .scale = tVec3f(8000.f),
      .material = tVec4f(0.6f, 0, 0, 0)
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

  placeable_mesh_assets.push_back({
    .mesh_name = "arch_1",
    .mesh_index = meshes.arch_1,
    .placeholder = true,
    .defaults = {
      .scale = tVec3f(1500000.f),
      .material = tVec4f(0.9f, 0, 0, 0.3f)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "station_drone",
    .mesh_index = meshes.station_drone,
    .placeholder = true,
    .defaults = {
      .scale = tVec3f(2500.f),
      .material = tVec4f(0.9f, 0, 0, 0.2f)
    }
  });

  placeable_mesh_assets.push_back({
    .mesh_name = "background_ship_1",
    .mesh_index = meshes.background_ship_1,
    .defaults = {
      .scale = tVec3f(100000.f),
      .material = tVec4f(0.9f, 0, 0, 0.2f)
    }
  });
}

static void LoadGeneratedMeshes(Tachyon* tachyon, State& state) {
  #define load_mesh(__name) meshes.__name = Tachyon_AddMesh(tachyon, Tachyon_LoadMesh("./cosmodrone/assets/station-parts/generated/" #__name ".obj"), 3000)

  #define load_mesh_with_2_lods(__name) meshes.__name =\
    Tachyon_AddMesh(\
      tachyon,\
      Tachyon_LoadMesh("./cosmodrone/assets/station-parts/generated/" #__name ".obj"),\
      Tachyon_LoadMesh("./cosmodrone/assets/station-parts/generated/" #__name "_lod_2.obj"),\
      3000\
    )

  auto& meshes = state.meshes;

  meshes.gas_flare_1 = Tachyon_AddMesh(tachyon, Tachyon_CreateSphereMesh(16), 100);

  load_mesh(antenna_2_frame);
  load_mesh(antenna_2_receivers);

  load_mesh(antenna_4_base);
  load_mesh(antenna_4_dish);

  load_mesh_with_2_lods(silo_3_body);
  load_mesh_with_2_lods(silo_3_frame);

  load_mesh(silo_6_body);
  load_mesh(silo_6_frame);
  load_mesh(silo_6_pipes);

  load_mesh(silo_7_core);
  load_mesh(silo_7_frame);

  load_mesh_with_2_lods(girder_4_core);
  load_mesh_with_2_lods(girder_4_frame);

  load_mesh(girder_6_core);
  load_mesh_with_2_lods(girder_6_frame);

  load_mesh(beam_2_core);
  load_mesh(beam_2_frame);

  load_mesh_with_2_lods(habitation_1_core);
  load_mesh_with_2_lods(habitation_1_frame);
  load_mesh_with_2_lods(habitation_1_insulation);

  load_mesh(habitation_2_body);
  load_mesh(habitation_2_frame);

  load_mesh(habitation_3_core);
  load_mesh_with_2_lods(habitation_3_frame);
  load_mesh(habitation_3_lights);

  load_mesh(habitation_4_body);
  load_mesh(habitation_4_core);
  load_mesh(habitation_4_frame);
  load_mesh(habitation_4_panels);
  load_mesh(habitation_4_lights);

  load_mesh(module_2_core);
  load_mesh(module_2_frame);

  load_mesh(solar_panel_2_cells);
  load_mesh(solar_panel_2_frame);

  load_mesh(elevator_torus_1_frame);

  load_mesh(station_torus_2_body);
  load_mesh(station_torus_2_supports);
  load_mesh(station_torus_2_frame);

  load_mesh(station_torus_3_body);
  load_mesh(station_torus_3_frame);
  load_mesh(station_torus_3_lights);

  load_mesh(station_torus_4_body);
  load_mesh(station_torus_4_frame);

  load_mesh(platform_body);
  load_mesh(platform_torus);
  load_mesh(platform_frame);
  load_mesh(platform_supports);

  load_mesh(light_1_base);
  load_mesh(light_1_bulb);

  load_mesh(light_2_base);
  load_mesh(light_2_bulb);

  load_mesh(light_3_base);
  load_mesh_with_2_lods(light_3_bulb);

  load_mesh(light_4_base);
  load_mesh(light_4_bulb);

  load_mesh(arch_1_body);
  load_mesh(arch_1_frame);
  load_mesh(arch_1_details);

  load_mesh(track_1_frame);

  load_mesh(station_drone_core);
  load_mesh(station_drone_frame);
  load_mesh(station_drone_rotator);
  load_mesh(station_drone_light);

  load_mesh(floater_1_core);
  load_mesh(floater_1_base);
  load_mesh(floater_1_frame);
  load_mesh(floater_1_spokes);
  load_mesh(floater_1_panels);

  load_mesh(fighter_core);
  load_mesh(fighter_frame);
  load_mesh(fighter_dock);
  load_mesh(fighter_guns);
  load_mesh(fighter_thrusters);

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

    // antenna_4
    {
      .mesh_index = meshes.antenna_4_base,
      .generated_from = meshes.antenna_4,
      .defaults = {
        .material = tVec4f(1.f, 0.5f, 0, 0.2f)
      }
    },
    {
      .mesh_index = meshes.antenna_4_dish,
      .generated_from = meshes.antenna_4,
      .defaults = {
        .material = tVec4f(0.5f, 0, 0, 0.4f)
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
        .color = tVec3f(0.6f, 0.4f, 0.4f),
        .material = tVec4f(0.6f, 0.5f, 0.1f, 0)
      }
    },
    {
      .mesh_index = meshes.silo_6_frame,
      .generated_from = meshes.silo_6,
      .defaults = {
        .color = tVec3f(0.7f),
        .material = tVec4f(0.2f, 1.f, 0, 0)
      }
    },
    {
      .mesh_index = meshes.silo_6_pipes,
      .generated_from = meshes.silo_6,
      .defaults = {
        .color = tVec3f(0.4f, 0.1f, 0.1f),
        .material = tVec4f(0.6f, 1.f, 0, 0)
      }
    },

    // silo_7
    {
      .mesh_index = meshes.silo_7_core,
      .generated_from = meshes.silo_7,
      .defaults = {
        .color = tVec3f(1.f, 0.6f, 0.5f),
        .material = tVec4f(1.f, 0, 0, 0.2f)
      }
    },
    {
      .mesh_index = meshes.silo_7_frame,
      .generated_from = meshes.silo_7,
      .defaults = {
        .color = tVec3f(0.8f),
        .material = tVec4f(0.7f, 1.f, 0, 0)
      }
    },

    // girder_4
    {
      .mesh_index = meshes.girder_4_core,
      .generated_from = meshes.girder_4,
      .defaults = {
        .color = tVec3f(0.6f),
        .material = tVec4f(0.7f, 1.f, 0, 0)
      }
    },
    {
      .mesh_index = meshes.girder_4_frame,
      .generated_from = meshes.girder_4,
      .defaults = {
        .color = tVec3f(1.f),
        .material = tVec4f(0.6f, 1.f, 0, 0.5f)
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

    // beam_2
    {
      .mesh_index = meshes.beam_2_core,
      .generated_from = meshes.beam_2,
      .defaults = {
        .color = tVec3f(1.f, 0.3f, 0.2f),
        .material = tVec4f(0.8f, 0, 0, 0.1f)
      }
    },
    {
      .mesh_index = meshes.beam_2_frame,
      .generated_from = meshes.beam_2,
      .defaults = {
        .material = tVec4f(0.9f, 0, 0, 0.1f)
      }
    },

    // habitation_1,
    {
      .mesh_index = meshes.habitation_1_core,
      .generated_from = meshes.habitation_1,
      .defaults {
        .color = tVec3f(1.f),
        .material = tVec4f(0.6f, 0.5f, 0, 0.4f)
      }
    },
    {
      .mesh_index = meshes.habitation_1_frame,
      .generated_from = meshes.habitation_1,
      .defaults {
        .color = tVec3f(1.f, 0.9f, 0.8f),
        .material = tVec4f(0.4f, 1.f, 0, 0)
      }
    },
    {
      .mesh_index = meshes.habitation_1_insulation,
      .generated_from = meshes.habitation_1,
      .defaults {
        .color = tVec3f(0.8f, 0.9f, 1.f),
        .material = tVec4f(0.2f, 0.7f, 0, 0)
      }
    },

    // habitation_2
    {
      .mesh_index = meshes.habitation_2_body,
      .generated_from = meshes.habitation_2,
      .defaults {
        .color = tVec3f(0.2f, 0.4f, 1.f),
        .material = tVec4f(0.4f, 1.f, 0, 0)
      }
    },
    {
      .mesh_index = meshes.habitation_2_frame,
      .generated_from = meshes.habitation_2,
      .defaults {
        .color = tVec3f(1.f),
        .material = tVec4f(0.8f, 0.f, 0, 0.5f)
      }
    },

    // habitation_3
    {
      .mesh_index = meshes.habitation_3_core,
      .generated_from = meshes.habitation_3,
      .defaults {
        .color = tVec3f(0.9f),
        .material = tVec4f(0.6f, 0, 0, 0.3f)
      }
    },
    {
      .mesh_index = meshes.habitation_3_frame,
      .generated_from = meshes.habitation_3,
      .defaults {
        .color = tVec3f(1.f, 0.7f, 0.2f),
        .material = tVec4f(0.5f, 1.f, 0, 0)
      }
    },
    {
      .mesh_index = meshes.habitation_3_lights,
      .generated_from = meshes.habitation_3,
      .defaults {
        .color = tVec4f(1.f, 0.9f, 0.9f, 1.f),
        .material = tVec4f(1., 0, 0, 0)
      }
    },

    // habitation_4
    {
      .mesh_index = meshes.habitation_4_body,
      .generated_from = meshes.habitation_4,
      .defaults {
        .color = tVec3f(1.f),
        .material = tVec4f(0.6f, 0, 0, 0.4f)
      }
    },
    {
      .mesh_index = meshes.habitation_4_core,
      .generated_from = meshes.habitation_4,
      .defaults {
        .color = tVec3f(0.7f),
        .material = tVec4f(0.4f, 0.5f, 0, 0.2f)
      }
    },
    {
      .mesh_index = meshes.habitation_4_frame,
      .generated_from = meshes.habitation_4,
      .defaults {
        .color = tVec3f(1.f),
        .material = tVec4f(0.2f, 1.f, 0, 0)
      }
    },
    {
      .mesh_index = meshes.habitation_4_panels,
      .generated_from = meshes.habitation_4,
      .defaults {
        .color = tVec3f(0.2f, 0.4f, 1.f),
        .material = tVec4f(0.2f, 1.f, 0.3f, 0)
      }
    },
    {
      .mesh_index = meshes.habitation_4_lights,
      .generated_from = meshes.habitation_4,
      .defaults {
        .color = tVec4f(1.f, 0.9f, 0.9f, 1.f),
        .material = tVec4f(1.f, 0, 0, 0)
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
        .color = 0x22F1,
        .material = tVec4f(0.2f, 1.f, 0.3f, 1.f) 
      }
    },
    {
      .mesh_index = meshes.solar_panel_2_frame,
      .generated_from = meshes.solar_panel_2,
      .defaults = {
        .material = tVec4f(0.6f, 1.f, 0, 0)
      }
    },

    // elevator_torus_1_frame
    {
      .mesh_index = meshes.elevator_torus_1_frame,
      .generated_from = meshes.elevator_torus_1,
      .defaults = {
        .color = tVec3f(1.f, 0.5f, 0.5f),
        .material = tVec4f(0.4f, 1.f, 0, 0.2f)
      }
    },

    // station_torus_2
    {
      .mesh_index = meshes.station_torus_2_body,
      .generated_from = meshes.station_torus_2,
      .defaults = {
        .material = tVec4f(0.7f, 0, 0, 0.2f)
      }
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
        .material = tVec4f(0.7f, 0, 0, 0.2f)
      }
    },
    {
      .mesh_index = meshes.station_torus_3_frame,
      .generated_from = meshes.station_torus_3,
      .defaults = {
        .material = tVec4f(0.1f, 1.f, 0, 0)
      }
    },
    {
      .mesh_index = meshes.station_torus_3_lights,
      .generated_from = meshes.station_torus_3,
      .defaults = {
        .color = tVec4f(1.f, 0.9f, 0.9f, 1.f),
        .material = tVec4f(1., 0, 0, 0)
      }
    },

    // station_torus_4
    {
      .mesh_index = meshes.station_torus_4_body,
      .generated_from = meshes.station_torus_4,
      .defaults = {
        .material = tVec4f(0.9f, 0, 0, 0.2f)
      }
    },
    {
      .mesh_index = meshes.station_torus_4_frame,
      .generated_from = meshes.station_torus_4,
      .defaults = {
        .material = tVec4f(0.4f, 1.f, 0, 0)
      }
    },

    // platform
    {
      .mesh_index = meshes.platform_body,
      .generated_from = meshes.platform,
      .defaults = {
        .material = tVec4f(1.f, 0, 0, 0.2f)
      }
    },
    {
      .mesh_index = meshes.platform_torus,
      .generated_from = meshes.platform
    },
    {
      .mesh_index = meshes.platform_frame,
      .generated_from = meshes.platform,
      .defaults {
        .color = tVec3f(1.f, 0.3f, 0.1f),
        .material = tVec4f(0.5f, 0, 0, 0.6f)
      }
    },
    {
      .mesh_index = meshes.platform_supports,
      .generated_from = meshes.platform,
      .defaults {
        .color = tVec3f(1.f),
        .material = tVec4f(0.7f, 1.f, 0, 0)
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

    // light_4
    {
      .mesh_index = meshes.light_4_base,
      .generated_from = meshes.light_4,
      .defaults = {
        .material = tVec4f(0.7f, 1.f, 0, 0)
      }
    },
    {
      .mesh_index = meshes.light_4_bulb,
      .generated_from = meshes.light_4,
      .defaults = {
        .color = tVec4f(1.f, 0.6f, 0.2f, 1.f),
        .material = tVec4f(1.f, 0, 0, 0)
      }
    },

    // arch_1
    {
      .mesh_index = meshes.arch_1_body,
      .generated_from = meshes.arch_1,
      .defaults {
        .color = tVec3f(0.6f),
        .material = tVec4f(0.8f, 0, 0, 0.4f)
      }
    },
    {
      .mesh_index = meshes.arch_1_frame,
      .generated_from = meshes.arch_1,
      .defaults {
        .color = tVec3f(0.8f),
        .material = tVec4f(0.3f, 1.f, 0, 0)
      }
    },
    {
      .mesh_index = meshes.arch_1_details,
      .generated_from = meshes.arch_1,
      .defaults {
        .color = tVec3f(0.5f),
        .material = tVec4f(0.8f, 0, 0, 0.2f)
      }
    },

    // track_1
    {
      .mesh_index = meshes.track_1_frame,
      .generated_from = meshes.procedural_track_1,
      .defaults = {
        .material = tVec4f(0.4f, 1.f, 0, 0)
      }
    },

    // station_drone
    {
      .mesh_index = meshes.station_drone_core,
      .generated_from = meshes.station_drone,
      .defaults = {
        .material = tVec4f(0.1f, 1.f, 0, 0)
      }
    },
    {
      .mesh_index = meshes.station_drone_frame,
      .generated_from = meshes.station_drone,
      .defaults = {
        .material = tVec4f(0.6f, 0, 0, 0.2f)
      }
    },
    {
      .mesh_index = meshes.station_drone_rotator,
      .generated_from = meshes.station_drone,
      .defaults = {
        .material = tVec4f(0.9f, 0, 0, 0.1f)
      }
    },
    {
      .mesh_index = meshes.station_drone_light,
      .generated_from = meshes.station_drone,
      .defaults = {
        .color = tVec4f(0.1f, 0.6f, 1.f, 1.f),
        .material = tVec4f(0.1f, 1.f, 0, 0)
      }
    },

    // floater_1
    {
      .mesh_index = meshes.floater_1_core,
      .generated_from = meshes.floater_1,
      .defaults = {
        .material = tVec4f(0.9f, 0, 1.f, 0.4f)
      }
    },
    {
      .mesh_index = meshes.floater_1_base,
      .generated_from = meshes.floater_1
    },
    {
      .mesh_index = meshes.floater_1_frame,
      .generated_from = meshes.floater_1,
      .defaults = {
        .material = tVec4f(0.2f, 1.f, 0, 0)
      }
    },
    {
      .mesh_index = meshes.floater_1_spokes,
      .generated_from = meshes.floater_1,
      .defaults = {
        .material = tVec4f(0.6f, 1.f, 0, 0)
      }
    },
    {
      .mesh_index = meshes.floater_1_panels,
      .generated_from = meshes.floater_1,
      .defaults = {
        .color = 0x44F1,
        .material = tVec4f(0.2f, 1.f, 0.3f, 1.f)
      }
    },

    // fighter_spawn
    {
      .mesh_index = meshes.fighter_core,
      .generated_from = meshes.fighter_spawn,
      .defaults = {
        .material = tVec4f(0.9f, 0, 0.2f, 0.2f)
      }
    },
    {
      .mesh_index = meshes.fighter_frame,
      .generated_from = meshes.fighter_spawn,
      .defaults = {
        .material = tVec4f(1.f, 0.2f, 0, 0)
      }
    },
    {
      .mesh_index = meshes.fighter_dock,
      .generated_from = meshes.fighter_spawn,
      .defaults = {
        .material = tVec4f(1.f, 0.5f, 0, 0)
      }
    },
    {
      .mesh_index = meshes.fighter_guns,
      .generated_from = meshes.fighter_spawn,
      .defaults = {
        .material = tVec4f(0.3f, 1.f, 0, 0)
      }
    },
    {
      .mesh_index = meshes.fighter_thrusters,
      .generated_from = meshes.fighter_spawn,
      .defaults = {
        .color = tVec3f(0.3f),
        .material = tVec4f(1.f, 0, 0, 0.1f)
      }
    },

    // gas_flare_1,
    {
      .mesh_index = meshes.gas_flare_1,
      .generated_from = meshes.gas_flare_1_spawn,
      .defaults = {
        .type = FIRE_MESH,
        .scale = tVec3f(6000.f, 18000.f, 6000.f),
        .color = tVec4f(1.f, 0.2f, 0.1f, 0.8f)
      }
    },
  };
}

static void LoadTargetInspectorMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  #define load_wireframe(mesh_index, obj_path)\
    mesh_index = Tachyon_AddMesh(tachyon, Tachyon_LoadMesh(obj_path), 1);\
    mesh(mesh_index).type = WIREFRAME_MESH\

  load_wireframe(meshes.drone_wireframe, "./cosmodrone/assets/wireframes/drone.obj");
  load_wireframe(meshes.antenna_3_wireframe, "./cosmodrone/assets/station-parts/antenna_3.obj");
  load_wireframe(meshes.floater_1_wireframe, "./cosmodrone/assets/station-parts/floater_1.obj");
  load_wireframe(meshes.station_drone_wireframe, "./cosmodrone/assets/station-parts/station_drone.obj");
  load_wireframe(meshes.fighter_wireframe, "./cosmodrone/assets/station-parts/fighter_spawn.obj");
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
    auto flight_curve = Tachyon_LoadMesh("./cosmodrone/assets/flight_curve.obj");
    auto beacon = Tachyon_LoadMesh("./cosmodrone/assets/beacon.obj");

    meshes.hud_flight_arrow = Tachyon_AddMesh(tachyon, flight_arrow, 16);
    meshes.hud_flight_curve = Tachyon_AddMesh(tachyon, flight_curve, 16);
    meshes.beacon = Tachyon_AddMesh(tachyon, beacon, 500);

    mesh(meshes.hud_flight_arrow).type = WIREFRAME_MESH;
    mesh(meshes.hud_flight_curve).type = WIREFRAME_MESH;
    mesh(meshes.beacon).type = WIREFRAME_MESH;
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

  BackgroundVehicles::LoadVehicleMeshes(tachyon, state);
  ProceduralGeneration::LoadMeshes(tachyon, state);

  LoadShipPartMeshes(tachyon, state);
  LoadPlaceableMeshes(tachyon, state);
  LoadGeneratedMeshes(tachyon, state);
  LoadTargetInspectorMeshes(tachyon, state);
  LoadBackgroundMeshes(tachyon, state);
  LoadEntityMeshes(tachyon, state);
  LoadDebugMeshes(tachyon, state);

  // Set placeable/generated mesh types
  {
    for (auto& asset : placeable_mesh_assets) {
      mesh(asset.mesh_index).type = asset.defaults.type;
    }

    for (auto& asset : generated_mesh_assets) {
      mesh(asset.mesh_index).type = asset.defaults.type;
    }
  }

  // @todo dev mode only
  {
    auto load_time = (Tachyon_GetMicroseconds() - start_time) / 1000;

    add_console_message("Loaded meshes in " + std::to_string(load_time) + "ms", tVec3f(1.f));
  }

  // Define highest cascades per mesh
  {
    auto& records = tachyon->mesh_pack.mesh_records;
    auto& meshes = state.meshes;

    mesh(meshes.grate_2).shadow_cascade_ceiling = 3;
    mesh(meshes.grate_3).shadow_cascade_ceiling = 3;
    mesh(meshes.girder_1).shadow_cascade_ceiling = 3;
    mesh(meshes.girder_1b).shadow_cascade_ceiling = 3;
    mesh(meshes.girder_4_frame).shadow_cascade_ceiling = 3;
    mesh(meshes.antenna_1).shadow_cascade_ceiling = 3;
    mesh(meshes.antenna_2_frame).shadow_cascade_ceiling = 3;
    mesh(meshes.antenna_4_base).shadow_cascade_ceiling = 3;
    mesh(meshes.antenna_4_dish).shadow_cascade_ceiling = 3;
    mesh(meshes.silo_6_pipes).shadow_cascade_ceiling = 3;
    mesh(meshes.solar_panel_2_frame).shadow_cascade_ceiling = 3;

    mesh(meshes.grate_1).shadow_cascade_ceiling = 2;
    mesh(meshes.habitation_3_frame).shadow_cascade_ceiling = 2;
    mesh(meshes.light_3_base).shadow_cascade_ceiling = 2;
    mesh(meshes.light_3_bulb).shadow_cascade_ceiling = 2;
    mesh(meshes.station_drone_core).shadow_cascade_ceiling = 2;
    mesh(meshes.station_drone_frame).shadow_cascade_ceiling = 2;
    mesh(meshes.station_drone_light).shadow_cascade_ceiling = 2;
    mesh(meshes.station_drone_rotator).shadow_cascade_ceiling = 2;

    mesh(meshes.silo_3_frame).shadow_cascade_ceiling = 1;

    // Disable shadows for the following meshes
    {
      // Background meshes
      mesh(meshes.planet).shadow_cascade_ceiling = 0;
      mesh(meshes.space_elevator).shadow_cascade_ceiling = 0;

      // Station parts
      mesh(meshes.light_1_base).shadow_cascade_ceiling = 0;
      mesh(meshes.light_1_bulb).shadow_cascade_ceiling = 0;
      mesh(meshes.light_2_base).shadow_cascade_ceiling = 0;
      mesh(meshes.light_2_bulb).shadow_cascade_ceiling = 0;
      mesh(meshes.habitation_1_frame).shadow_cascade_ceiling = 0;
      mesh(meshes.habitation_1_insulation).shadow_cascade_ceiling = 0;
      mesh(meshes.habitation_2_frame).shadow_cascade_ceiling = 0;
      mesh(meshes.habitation_3_lights).shadow_cascade_ceiling = 0;
      mesh(meshes.habitation_4_panels).shadow_cascade_ceiling = 0;
      mesh(meshes.habitation_4_lights).shadow_cascade_ceiling = 0;
      mesh(meshes.station_torus_3_lights).shadow_cascade_ceiling = 0;

      // @todo dev mode only
      mesh(meshes.editor_guideline).shadow_cascade_ceiling = 0;
      mesh(meshes.cube).shadow_cascade_ceiling = 0;
    }
  }

  Tachyon_InitializeObjects(tachyon);
}

const std::vector<MeshAsset>& MeshLibrary::GetPlaceableMeshAssets() {
  return placeable_mesh_assets;
}


const std::vector<MeshAsset>& MeshLibrary::GetGeneratedMeshAssets() {
  return generated_mesh_assets;
}

// @todo optimize
const MeshAsset& MeshLibrary::FindMeshAsset(uint16 mesh_index) {
  for (auto& asset : placeable_mesh_assets) {
    if (asset.mesh_index == mesh_index) {
      return asset;
    }
  }

  for (auto& asset : generated_mesh_assets) {
    if (asset.mesh_index == mesh_index) {
      return asset;
    }
  }

  return MeshAsset();
}