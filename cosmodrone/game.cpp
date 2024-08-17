#include "cosmodrone/game.h"

static uint32 sun_index = 0;
static uint32 moon_index = 0;

void Cosmodrone::StartGame(Tachyon* tachyon) {
  auto sunMesh = Tachyon_LoadMesh("./cosmodrone/assets/sun-sign.obj");
  auto moonMesh = Tachyon_LoadMesh("./cosmodrone/assets/moon-sign.obj");

  sun_index = Tachyon_AddMesh(tachyon, sunMesh, 10);
  moon_index = Tachyon_AddMesh(tachyon, moonMesh, 10);

  create(sun_index);
  create(sun_index);

  create(moon_index);
  create(moon_index);

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

  // @temporary
  {
    auto& moon = objects(moon_index)[0];
    auto& moon2 = objects(moon_index)[1];

    moon.position = tVec3f(-300.f, 50.f * cosf(tachyon->running_time * 0.2f), -200.f);
    moon.scale = tVec3f(40.f);
    moon.rotation = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), 0.05f * tachyon->running_time);
    moon.color = tVec4f(1.f, 0, 0.f, 0);

    moon2.position = tVec3f(300.f, 50.f * sinf(tachyon->running_time * 0.2f), -200.f);
    moon2.scale = tVec3f(40.f);
    moon2.rotation = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), -0.05f * tachyon->running_time);
    moon2.color = tVec4f(0, 0, 1.f, 0);

    commit(moon);
    commit(moon2);
  }
}