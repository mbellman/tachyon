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

// @todo use within function scopes
static struct State {
  tCamera3p camera3p;

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

static void UpdateShip(Tachyon* tachyon, State& state, float dt) {
  auto& hull = objects(meshes.hull)[0];
  auto& streams = objects(meshes.streams)[0];
  auto& thrusters = objects(meshes.thrusters)[0];
  auto& trim = objects(meshes.trim)[0];

  // @todo add an engine constant for pi/tau/half pi
  float pi = 3.141592f;
  float yaw = atan2f(state.ship_velocity.x, state.ship_velocity.z) + pi;

  float pitch = state.ship_velocity.magnitude() > 0.f
    ? state.ship_velocity.unit().y * pi/2.f
    : 0.f;

  auto rotation = (
    Quaternion::fromAxisAngle(tVec3f(0, 1.f, 0), yaw) *
    Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), pitch)
  );

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
  auto sphereMesh = Tachyon_CreateSphereMesh(8);

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

    state.camera3p.azimuth += (float)tachyon->mouse_delta_x / 1000.f;
    state.camera3p.altitude += (float)tachyon->mouse_delta_y / 1000.f;
  }

  state.camera3p.radius = 1000.f;

  camera.position = state.camera3p.calculatePosition();

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
    auto direction = camera.orientation.getDirection();

    if (is_key_held(tKey::W)) {
      state.ship_velocity += direction * (500.f * dt);
    }

    state.ship_position += state.ship_velocity * dt;

    UpdateShip(tachyon, state, dt);
  }

  tVec3f camera_look_at_target = state.ship_position;

  camera.position = camera_look_at_target + state.camera3p.calculatePosition();

  // @todo LookAtTarget(const tVec3f& target)
  {
    tVec3f forward = (camera_look_at_target - camera.position).unit();
    tVec3f sideways = tVec3f::cross(forward, tVec3f(0, 1.f, 0));
    auto upsideDown = state.camera3p.isUpsideDown();

    tVec3f up = upsideDown
      ? tVec3f::cross(forward, sideways)
      : tVec3f::cross(sideways, forward);

    camera.orientation.face(forward, up);
  }
}