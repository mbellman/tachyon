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

  // Swiveling (azimuth)
  {
    const float swivel_speed = 3.f;

    camera3p.azimuth += tachyon->right_stick.x * swivel_speed * state.dt;
  }

  // Zooming in/out (altitude)
  {
    const float min = 0.1f;
    const float max = 1.2f;
    const float zoom_speed = 1.5f;

    camera3p.altitude += tachyon->right_stick.y * zoom_speed * state.dt;

    if (camera3p.altitude < min) camera3p.altitude = min;
    if (camera3p.altitude > max) camera3p.altitude = max;

    float radiusAlpha = Tachyon_InverseLerp(min, max, camera3p.altitude);

    camera3p.radius = 10000.f + 15000.f * radiusAlpha;
  }

  tVec3f target = tVec3f(0, 0, -10000.f);
  tVec3f updated_position = target + camera3p.calculatePosition();

  camera.position = tVec3f::lerp(camera.position, updated_position, 8.f * state.dt);

  LookAt(camera, target);
}