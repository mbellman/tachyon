#include "cosmodrone/game.h"

static uint32 sun_index = 0;

void Cosmodrone::StartGame(Tachyon* tachyon) {
  auto mesh = Tachyon_LoadMesh("./cosmodrone/assets/test.obj");

  sun_index = Tachyon_AddMesh(tachyon, mesh, 10);

  create(sun_index);
  create(sun_index);

  Tachyon_InitializeObjects(tachyon);
}

void Cosmodrone::RunGame(Tachyon* tachyon) {
  // @temporary
  {
    auto& sun = objects(sun_index)[0];
    auto& sun2 = objects(sun_index)[1];

    sun.position = tVec3f(-100.f, 50.f * sinf(tachyon->running_time * 0.2f), -200.f);
    sun.scale = tVec3f(40.f);
    sun.rotation = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), 0.05f * tachyon->running_time);
    sun.color = tVec4f(1.f, 0, 0.f, 0);

    sun2.position = tVec3f(100.f, 50.f * cosf(tachyon->running_time * 0.2f), -200.f);
    sun2.scale = tVec3f(40.f);
    sun2.rotation = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), -0.05f * tachyon->running_time);
    sun2.color = tVec4f(0, 0, 1.f, 0);

    commit(sun);
    commit(sun2);
  }
}