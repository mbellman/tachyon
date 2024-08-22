#include "cosmodrone/game.h"

static uint32 sun_index = 0;
static uint32 moon_index = 0;

void Cosmodrone::StartGame(Tachyon* tachyon) {
  auto sunMesh = Tachyon_LoadMesh("./cosmodrone/assets/sun-sign.obj", tVec3f(-1.f, 1.f, 1.f));
  auto moonMesh = Tachyon_LoadMesh("./cosmodrone/assets/moon-sign.obj", tVec3f(-1.f, 1.f, 1.f));

  sun_index = Tachyon_AddMesh(tachyon, sunMesh, 10);
  moon_index = Tachyon_AddMesh(tachyon, moonMesh, 10);

  create(sun_index);
  create(sun_index);

  create(moon_index);
  create(moon_index);

  Tachyon_InitializeObjects(tachyon);
}

void Cosmodrone::RunGame(Tachyon* tachyon) {
  auto& scene = tachyon->scene;
  auto& camera = scene.camera;

  if (tachyon->is_window_focused) {
    camera.orientation.yaw += (float)tachyon->mouse_delta_x / 1000.f;
    camera.orientation.pitch += (float)tachyon->mouse_delta_y / 1000.f;
  }

  if (camera.orientation.pitch > 0.9f) camera.orientation.pitch = 0.9f;
  else if (camera.orientation.pitch < -0.9f) camera.orientation.pitch = -0.9f;

  if (is_key_held(tKey::W)) {
    camera.position += camera.orientation.getDirection() * 0.5f;
  } else if (is_key_held(tKey::S)) {
    camera.position += camera.orientation.getDirection() * -0.5f;
  }

  if (is_key_held(tKey::A)) {
    camera.position += camera.orientation.getLeftDirection() * 0.5f;
  } else if (is_key_held(tKey::D)) {
    camera.position += camera.orientation.getRightDirection() * 0.5f;
  }

  // @temporary
  {
    auto& sun = objects(sun_index)[0];
    auto& sun2 = objects(sun_index)[1];

    sun.position = tVec3f(-100.f, 50.f * sinf(tachyon->running_time * 3.f), -800.f);
    sun.scale = tVec3f(40.f);
    sun.rotation = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), tachyon->running_time);
    sun.color = tVec4f(1.f, 0, 0.f, 0);

    sun2.position = tVec3f(100.f, 50.f * cosf(tachyon->running_time * 3.f), -800.f);
    sun2.scale = tVec3f(40.f);
    sun2.rotation = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), -tachyon->running_time);
    sun2.color = tVec4f(0, 0, 1.f, 0);

    commit(sun);
    commit(sun2);
  }

  // @temporary
  {
    auto& moon = objects(moon_index)[0];
    auto& moon2 = objects(moon_index)[1];

    moon.position = tVec3f(-300.f, 50.f * cosf(tachyon->running_time * 3.f), -800.f);
    moon.scale = tVec3f(40.f);
    moon.rotation = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), tachyon->running_time);
    moon.color = tVec4f(1.f, 0, 0.f, 0);

    moon2.position = tVec3f(300.f, 50.f * sinf(tachyon->running_time * 3.f), -800.f);
    moon2.scale = tVec3f(40.f);
    moon2.rotation = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), -tachyon->running_time);
    moon2.color = tVec4f(0, 0, 1.f, 0);

    commit(moon);
    commit(moon2);
  }
}