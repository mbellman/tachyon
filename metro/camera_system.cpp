#include "engine/tachyon.h"

#include "metro/camera_system.h"

using namespace metro;

static void LookAt(tCamera& camera, const tVec3f& position) {
  tVec3f direction = (position - camera.position).unit();

  camera.orientation.face(direction, tVec3f(0, 1.f, 0));
  camera.rotation = camera.orientation.toQuaternion();
}

void CameraSystem::Update(Tachyon* tachyon, State& state) {
  auto& camera3p = tachyon->scene.camera3p;
  auto& camera = tachyon->scene.camera;

  // Swiveling
  {
    camera3p.azimuth += tachyon->right_stick.x * 2.f * state.dt;
  }

  // Zooming in/out
  {
    camera3p.altitude += tachyon->right_stick.y * state.dt;

    if (camera3p.altitude < 0.1f) camera3p.altitude = 0.1f;

    camera3p.limitAltitude(0.9f);

    // @temporary
    if (tachyon->right_stick.y > 0.f) {
      camera3p.radius = Tachyon_Lerpf(camera3p.radius, 25000.f, state.dt);
    } else if (tachyon->right_stick.y < 0.f) {
      camera3p.radius = Tachyon_Lerpf(camera3p.radius, 10000.f, state.dt);
    }
  }

  tVec3f target = tVec3f(0, 0, -10000.f);

  camera.position = target + camera3p.calculatePosition();

  LookAt(camera, target);
}