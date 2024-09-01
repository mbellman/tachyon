#include "cosmodrone/game.h"

static struct Meshes {
  uint32
    sun,
    moon,
    plane,
    sphere;
} meshes;

static void SetupFlightSim(Tachyon* tachyon) {
  for (int32 i = 0; i < 40; i++) {
    for (int32 j = 0; j < 40; j++) {
      for (int32 k = 0; k < 40; k++) {
        auto& sphere = create(meshes.sphere);

        sphere.position = tVec3f(
          (i - 20) * 4000.f,
          (j - 20) * 4000.f,
          (k - 20) * 4000.f
        );

        sphere.scale = 50.f;
        sphere.color = tVec3f(0.1f, 0.3f, 1.f);
        sphere.material = tVec4f(0.7f, 0.f, 1.f, 1.f);

        commit(sphere);
      }
    }
  }
}

void Cosmodrone::StartGame(Tachyon* tachyon) {
  auto sunMesh = Tachyon_LoadMesh("./cosmodrone/assets/sun-sign.obj", tVec3f(-1.f, 1.f, 1.f));
  auto moonMesh = Tachyon_LoadMesh("./cosmodrone/assets/moon-sign.obj", tVec3f(-1.f, 1.f, 1.f));
  auto planeMesh = Tachyon_CreatePlaneMesh();
  auto sphereMesh = Tachyon_CreateSphereMesh(8);



  meshes.sun = Tachyon_AddMesh(tachyon, sunMesh, 1);
  meshes.moon = Tachyon_AddMesh(tachyon, moonMesh, 1);
  meshes.plane = Tachyon_AddMesh(tachyon, planeMesh, 1);
  meshes.sphere = Tachyon_AddMesh(tachyon, sphereMesh, 40 * 40 * 40);

  Tachyon_InitializeObjects(tachyon);

  SetupFlightSim(tachyon);
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
}