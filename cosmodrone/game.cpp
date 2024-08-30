#include "cosmodrone/game.h"

static struct Meshes {
  uint32
    sun,
    moon,
    plane,
    sphere;
} meshes;

void Cosmodrone::StartGame(Tachyon* tachyon) {
  auto sunMesh = Tachyon_LoadMesh("./cosmodrone/assets/sun-sign.obj", tVec3f(-1.f, 1.f, 1.f));
  auto moonMesh = Tachyon_LoadMesh("./cosmodrone/assets/moon-sign.obj", tVec3f(-1.f, 1.f, 1.f));
  auto planeMesh = Tachyon_CreatePlaneMesh();
  auto sphereMesh = Tachyon_CreateSphereMesh(20);

  meshes.sun = Tachyon_AddMesh(tachyon, sunMesh, 10);
  meshes.moon = Tachyon_AddMesh(tachyon, moonMesh, 10);
  meshes.plane = Tachyon_AddMesh(tachyon, planeMesh, 1);
  meshes.sphere = Tachyon_AddMesh(tachyon, sphereMesh, 32);

  Tachyon_InitializeObjects(tachyon);

  create(meshes.sun);
  create(meshes.sun);

  create(meshes.moon);
  create(meshes.moon);

  {
    create(meshes.plane);
    auto& plane = objects(meshes.plane)[0];

    plane.position = tVec3f(0, -600.f, -800.f);
    plane.scale = tVec3f(1000.f, 1.f, 1000.f);
    plane.color = tVec4f(1.f, 0.4f, 0.2f, 1.f);

    commit(plane);
  }

  {
    for (int8 i = 0; i < 8; i++) {
      for (int8 j = 0; j < 4; j++) {
        auto& sphere = create(meshes.sphere);

        sphere.position = tVec3f(150.f * (i - 4) + 75.f, -300.f, 150.f * (j - 2) - 800.f + 75.f);
        sphere.scale = tVec3f(50.f);
        sphere.color = tVec4f(1.f, 0.1f, 0.1f, 1.f);

        commit(sphere);
      }
    }
  }
}

void Cosmodrone::RunGame(Tachyon* tachyon, const float dt) {
  auto& scene = tachyon->scene;
  auto& camera = scene.camera;

  if (tachyon->is_window_focused) {
    camera.orientation.yaw += (float)tachyon->mouse_delta_x / 1000.f;
    camera.orientation.pitch += (float)tachyon->mouse_delta_y / 1000.f;
  }

  if (camera.orientation.pitch > 0.99f) camera.orientation.pitch = 0.99f;
  else if (camera.orientation.pitch < -0.99f) camera.orientation.pitch = -0.99f;

  if (is_key_held(tKey::W)) {
    camera.position += camera.orientation.getDirection() * dt * 500.f;
  } else if (is_key_held(tKey::S)) {
    camera.position += camera.orientation.getDirection() * -dt * 500.f;
  }

  if (is_key_held(tKey::A)) {
    camera.position += camera.orientation.getLeftDirection() * dt * 500.f;
  } else if (is_key_held(tKey::D)) {
    camera.position += camera.orientation.getRightDirection() * dt * 500.f;
  }

  // @temporary
  {
    auto& sun = objects(meshes.sun)[0];
    auto& sun2 = objects(meshes.sun)[1];

    sun.position = tVec3f(-100.f, 50.f * sinf(tachyon->running_time * 3.f), -800.f);
    sun.scale = tVec3f(40.f);
    sun.rotation = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), tachyon->running_time);
    sun.color = tVec4f(1.f, 0, 0, 0);

    sun2.position = tVec3f(100.f, 50.f * cosf(tachyon->running_time * 3.f), -800.f);
    sun2.scale = tVec3f(40.f);
    sun2.rotation = Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), -tachyon->running_time);
    sun2.color = tVec4f(0, 0, 1.f, 0);

    commit(sun);
    commit(sun2);
  }

  // @temporary
  {
    auto& moon = objects(meshes.moon)[0];
    auto& moon2 = objects(meshes.moon)[1];

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