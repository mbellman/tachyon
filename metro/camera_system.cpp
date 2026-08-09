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

static CameraParams GetCameraParams(State& state, Bicycle* active_bike) {
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
  auto* active_bike = GetActiveBicycle(state);

  tVec3f last_move = state.player_position - state.previous_player_position;
  float last_move_distance = last_move.magnitude();

  bool was_just_manually_controlling_camera = (
    state.last_manual_camera_control_time != 0.f &&
    time_since(state.last_manual_camera_control_time) < 2.f
  );

  bool use_auto_centering = (
    !was_just_manually_controlling_camera &&
    last_move_distance > 0.f &&
    (state.player_bike_id != -1 || is_moving_left_stick())
  );

  // Tracking manual camera control time
  {
    if (is_moving_right_stick()) {
      state.last_manual_camera_control_time = get_scene_time();
      state.target_camera_azimuth = camera3p.azimuth;
    }
  }

  // Auto-centering behavior
  {
    if (use_auto_centering) {
      tVec3f forward_direction;

      if (active_bike != nullptr) {
        // If we're riding a bike, use the facing direction of the bike
        // to center the camera behind it
        forward_direction = active_bike->facing_direction;
      } else {
        // Otherwise, use our last movement direction
        // @todo use the player model direction?
        forward_direction = last_move / last_move_distance;
      }

      state.target_camera_azimuth = atan2f(forward_direction.z, forward_direction.x) + t_PI;

      state.target_camera_azimuth_blend_rate = Tachyon_Lerpf(
        state.target_camera_azimuth_blend_rate,
        1.f,
        state.dt
      );
    } else {
      state.target_camera_azimuth_blend_rate = Tachyon_Lerpf(
        state.target_camera_azimuth_blend_rate,
        0.f,
        5.f * state.dt
      );
    }
  }

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
    const float min = 0.f;
    const float max = 1.2f;
    const float zoom_speed = 1.5f;

    camera3p.altitude += tachyon->right_stick.y * zoom_speed * state.dt;

    float target_altitude = camera3p.altitude;

    if (use_auto_centering) {
      float alpha = state.recorded_player_speed / 10000.f;
      if (alpha > 1.f) alpha = 1.f;

      target_altitude = Tachyon_Lerpf(camera3p.altitude, 0.3f, alpha);
    }

    camera3p.altitude = Tachyon_Lerpf(
      camera3p.altitude,
      target_altitude,
      state.dt
    );

    if (camera3p.altitude < min) camera3p.altitude = min;
    if (camera3p.altitude > max) camera3p.altitude = max;

    float radius_alpha = Tachyon_InverseLerp(min, max, camera3p.altitude);

    camera3p.radius = 10000.f + 15000.f * radius_alpha;
  }

  auto params = GetCameraParams(state, active_bike);
  tVec3f target_camera_position = params.focus_point + camera3p.calculatePosition();

  camera.position = tVec3f::lerp(camera.position, target_camera_position, params.blend_rate * state.dt);

  // @todo smooth behavior when getting on/off bikes
  PointCameraAt(camera, params.focus_point);
}