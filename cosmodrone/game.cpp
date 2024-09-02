#include "cosmodrone/game.h"

static struct Meshes {
  uint32
    sun,
    moon,
    plane,
    sphere,

    hull,
    streams,
    thrusters,
    trim;
} meshes;

// @todo refactor
static struct FlightCamera {
  Quaternion rotation = Quaternion(1.f, 0, 0, 0);
  tVec3f forward = tVec3f(0, 0, -1.f);
  float radius = 1.f;
};

// @todo use within function scopes
static struct State {
  FlightCamera flight_camera;

  float current_ship_pitch = 0.f;
  float current_ship_yaw = 0.f;

  tVec3f ship_position;
  tVec3f ship_velocity;
} state;

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
        sphere.color = tVec3f(0.2f, 0.5f, 1.f);
        sphere.material = tVec4f(0.8f, 0.f, 1.f, 1.f);

        commit(sphere);
      }
    }
  }

  // @todo improve ship part handling
  {
    auto& hull = create(meshes.hull);
    auto& streams = create(meshes.streams);
    auto& thrusters = create(meshes.thrusters);
    auto& trim = create(meshes.trim);

    hull.scale = 200.f;
    hull.material = tVec4f(0.8f, 1.f, 0.2f, 0);

    streams.scale = 200.f;
    streams.material = tVec4f(0.6f, 0, 0, 1.f);

    thrusters.scale = 200.f;
    thrusters.color = tVec3f(0.2f);
    thrusters.material = tVec4f(0.8f, 0, 0, 0.4f);

    trim.scale = 200.f;
    trim.material = tVec4f(0.2f, 1.f, 0, 0);

    commit(hull);
    commit(streams);
    commit(thrusters);
    commit(trim);
  }
}

static void LoadTestShip(Tachyon* tachyon) {
  meshes.hull = Tachyon_AddMesh(tachyon, Tachyon_LoadMesh("./cosmodrone/assets/test-ship/hull.obj"), 1);
  meshes.streams = Tachyon_AddMesh(tachyon, Tachyon_LoadMesh("./cosmodrone/assets/test-ship/streams.obj"), 1);
  meshes.thrusters = Tachyon_AddMesh(tachyon, Tachyon_LoadMesh("./cosmodrone/assets/test-ship/thrusters.obj"), 1);
  meshes.trim = Tachyon_AddMesh(tachyon, Tachyon_LoadMesh("./cosmodrone/assets/test-ship/trim.obj"), 1);
}

// @todo move
static inline float Lerpf(float a, float b, float alpha) {
  return a + (b - a) * alpha;
}

// @todo move
static inline float LerpCircularF(float a, float b, float alpha, float maxRange) {
  float range = b - a;

  if (range > maxRange) {
    a += maxRange * 2.f;
  } else if (range < -maxRange) {
    a -= maxRange * 2.f;
  }

  return a + (b - a) * alpha;
}

static void UpdateShip(Tachyon* tachyon, State& state, float dt) {
  auto& camera = tachyon->scene.camera;

  auto& hull = objects(meshes.hull)[0];
  auto& streams = objects(meshes.streams)[0];
  auto& thrusters = objects(meshes.thrusters)[0];
  auto& trim = objects(meshes.trim)[0];

  Quaternion target_rotation = {
    camera.rotation.w,
    -camera.rotation.x,
    -camera.rotation.y,
    -camera.rotation.z
  };

  // @todo will nlerp work here?
  auto rotation = Quaternion::slerp(hull.rotation, target_rotation, dt);

  hull.position = state.ship_position;
  streams.position = state.ship_position;
  thrusters.position = state.ship_position;
  trim.position = state.ship_position;

  hull.rotation = streams.rotation = thrusters.rotation = trim.rotation = rotation;

  commit(hull);
  commit(streams);
  commit(thrusters);
  commit(trim);
}

void Cosmodrone::StartGame(Tachyon* tachyon) {
  auto sunMesh = Tachyon_LoadMesh("./cosmodrone/assets/sun-sign.obj", tVec3f(-1.f, 1.f, 1.f));
  auto moonMesh = Tachyon_LoadMesh("./cosmodrone/assets/moon-sign.obj", tVec3f(-1.f, 1.f, 1.f));
  auto planeMesh = Tachyon_CreatePlaneMesh();
  auto sphereMesh = Tachyon_CreateSphereMesh(4);

  meshes.sun = Tachyon_AddMesh(tachyon, sunMesh, 1);
  meshes.moon = Tachyon_AddMesh(tachyon, moonMesh, 1);
  meshes.plane = Tachyon_AddMesh(tachyon, planeMesh, 1);
  meshes.sphere = Tachyon_AddMesh(tachyon, sphereMesh, 40 * 40 * 40);

  LoadTestShip(tachyon);

  Tachyon_InitializeObjects(tachyon);

  SetupFlightSim(tachyon);
}

void Cosmodrone::RunGame(Tachyon* tachyon, const float dt) {
  auto& scene = tachyon->scene;
  auto& camera = scene.camera;

  if (tachyon->is_window_focused) {
    // @todo move this into a separate free-camera-mode handler
    // camera.orientation.yaw += (float)tachyon->mouse_delta_x / 1000.f;
    // camera.orientation.pitch += (float)tachyon->mouse_delta_y / 1000.f;

    // if (state.camera3p.isUpsideDown()) {
    //   state.camera3p.azimuth -= (float)tachyon->mouse_delta_x / 1000.f;
    // } else {
    //   state.camera3p.azimuth += (float)tachyon->mouse_delta_x / 1000.f;
    // }

    // @todo refactor
    auto up = tVec3f(0, 1.f, 0);
    auto left = tVec3f(-1.f, 0, 0);

    Quaternion turn = (
      Quaternion::fromAxisAngle(left, -(float)tachyon->mouse_delta_y / 1000.f) *
      Quaternion::fromAxisAngle(up, (float)tachyon->mouse_delta_x / 1000.f)
    );

    state.flight_camera.rotation = (turn * state.flight_camera.rotation).unit();

    camera.rotation = state.flight_camera.rotation;
  }

  // @todo refactor
  auto view_matrix = (
    camera.rotation.toMatrix4f() *
    tMat4f::translation(camera.position * tVec3f(-1.f))
  );

  tVec3f view_direction = tVec3f(
    view_matrix.m[8],
    view_matrix.m[9],
    view_matrix.m[10]
  ).invert();

  state.flight_camera.radius = 1000.f;

  // @todo move this into a separate free-camera-mode handler
  {
    // if (camera.orientation.pitch > 0.99f) camera.orientation.pitch = 0.99f;
    // else if (camera.orientation.pitch < -0.99f) camera.orientation.pitch = -0.99f;

    // if (is_key_held(tKey::W)) {
    //   camera.position += camera.orientation.getDirection() * dt * 500.f;
    // } else if (is_key_held(tKey::S)) {
    //   camera.position += camera.orientation.getDirection() * -dt * 500.f;
    // }

    // if (is_key_held(tKey::A)) {
    //   camera.position += camera.orientation.getLeftDirection() * dt * 500.f;
    // } else if (is_key_held(tKey::D)) {
    //   camera.position += camera.orientation.getRightDirection() * dt * 500.f;
    // }
  }

  // @todo move this into a controls handler
  {
    if (is_key_held(tKey::W)) {
      state.ship_velocity += view_direction * (500.f * dt);
    }

    state.ship_position += state.ship_velocity * dt;
  }

  tVec3f camera_look_at_target = state.ship_position;

  camera.position = camera_look_at_target - view_direction * 1000.f;

  // @todo move this into an animation handler
  {
    UpdateShip(tachyon, state, dt);
  }
}