#include "cosmodrone/vehicles.h"

using namespace Cosmodrone;

void Vehicles::LoadVehicleMeshes(Tachyon* tachyon, State& state) {
  auto& meshes = state.meshes;

  meshes.npc_drone_1 = Tachyon_AddMesh(
    tachyon,
    Tachyon_LoadMesh("./cosmodrone/assets/npc-ships/npc_drone_1.obj"),
    500
  );
}

void Vehicles::InitVehicles(Tachyon* tachyon, State& state) {
  // @todo
}

void Vehicles::UpdateVehicles(Tachyon* tachyon, State& state, const float dt) {
  // @todo
}