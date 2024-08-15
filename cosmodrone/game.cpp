#include "cosmodrone/game.h"

static uint32 sun_index = 0;

void Cosmodrone::StartGame(Tachyon* tachyon) {
  auto mesh = Tachyon_LoadMesh("./cosmodrone/assets/test.obj");

  sun_index = Tachyon_AddMesh(tachyon, mesh);

  Tachyon_InitializeObjects(tachyon);
}

void Cosmodrone::RunGame(Tachyon* tachyon) {
  // @temporary
  {
    auto& sun = objects(sun_index)[0];

    sun.position = tVec3f(0, 50.f * sinf(tachyon->running_time * 0.2f), -200.f);
    sun.scale = tVec3f(40.f);
    sun.rotation = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), 0.05f * tachyon->running_time);
    sun.color = tVec4f(1.f, 0, 0.f, 0);

    commit(sun);
  }
}