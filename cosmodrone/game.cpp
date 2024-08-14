#include "cosmodrone/game.h"

static uint32 sun_index = 0;

void Cosmodrone::StartGame(Tachyon* tachyon) {
  auto mesh = Tachyon_LoadMesh("./cosmodrone/assets/test.obj");

  sun_index = Tachyon_AddMesh(tachyon, mesh);

  Tachyon_InitializeObjects(tachyon);
}

void Cosmodrone::RunGame(Tachyon* tachyon) {
  // @temporary
  auto& sun = tachyon->mesh_pack.mesh_records[sun_index - 1].group[0];

  // printf("%f, %f, %f\n", sun.position.x, sun.position.y, sun.position.z);
  // printf("%d\n", tachyon->mesh_pack.mesh_records[sun_index - 1].group.total);
}