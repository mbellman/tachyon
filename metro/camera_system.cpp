#include "engine/tachyon.h"

#include "metro/camera_system.h"
#include "metro/utilities.h"

using namespace metro;

static void PointCameraAt(tCamera& camera, const tVec3f& target) {
  tVec3f direction = (target - camera.position).unit();

  camera.orientation.face(direction, tVec3f(0, 1.f, 0));
  camera.rotation = camera.orientation.toQuaternion();
}

struct CameraParams {
  tVec3f focus_point;
  float blend_rate;
};

static CameraParams GetCameraParams(State& state) {
  auto* active_bike = GetActiveBicycle(state);

  if (active_bike != nullptr) {
    return {
      .focus_point = active_bike->position + tVec3f(0, 3000.f, 0),
      .blend_rate = active_bike->in_freefall ? 16.f : 8.f
    };
  } else {
    return {
      .focus_point = state.player_position + tVec3f(0, 2000.f, 0),
      .blend_rate = 20.f
    };
  }
}

static void PerformAzimuthAdjustments(Tachyon* tachyon, State& state) {
  auto& camera3p = tachyon->scene.camera3p;

  if (abs(camera3p.azimuth - state.target_camera_azimuth) >= t_PI) {
    if (camera3p.azimuth > state.target_camera_azimuth) {
      state.target_camera_azimuth += t_TAU;
    } else {
      camera3p.azimuth += t_TAU;
    }
  }

  if (state.target_camera_azimuth > t_TAU) {
    state.target_camera_azimuth -= t_TAU;
    camera3p.azimuth -= t_TAU;
  }
}

void CameraSystem::Update(Tachyon* tachyon, State& state) {
  profile("CameraSystem::Update()");

  auto& camera3p = tachyon->scene.camera3p;
  auto& camera = tachyon->scene.camera;

  // Swiveling (azimuth)
  // @todo mouse support
  {
    const float swivel_speed = 3.f;

    camera3p.azimuth += tachyon->right_stick.x * swivel_speed * state.dt;

    PerformAzimuthAdjustments(tachyon, state);

    camera3p.azimuth = Tachyon_Lerpf(
      camera3p.azimuth,
      state.target_camera_azimuth,
      state.target_camera_azimuth_blend_rate * state.dt
    );
  }

  // Zooming in/out (altitude)
  // @todo mouse support
  {
    const float min = 0.1f;
    const float max = 1.2f;
    const float zoom_speed = 1.5f;

    camera3p.altitude += tachyon->right_stick.y * zoom_speed * state.dt;

    if (camera3p.altitude < min) camera3p.altitude = min;
    if (camera3p.altitude > max) camera3p.altitude = max;

    float radius_alpha = Tachyon_InverseLerp(min, max, camera3p.altitude);

    camera3p.radius = 10000.f + 15000.f * radius_alpha;
  }

  auto params = GetCameraParams(state);
  tVec3f target_camera_position = params.focus_point + camera3p.calculatePosition();

  camera.position = tVec3f::lerp(camera.position, target_camera_position, params.blend_rate * state.dt);

  PointCameraAt(camera, params.focus_point);
}