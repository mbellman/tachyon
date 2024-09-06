#include "cosmodrone/game.h"

static struct Meshes {
  uint32
    sun,
    moon,
    plane,
    sphere,
    cube,

    hull,
    streams,
    thrusters,
    trim;
} meshes;

static struct OrthonormalBasis {
  tVec3f forward;
  tVec3f up;
  tVec3f sideways;
};

// @todo use within function scopes
static struct State {
  Quaternion flight_camera_rotation = Quaternion(1.f, 0, 0, 0);

  tVec3f view_forward_direction;
  tVec3f view_up_direction;

  tVec3f ship_position;
  tVec3f ship_velocity;
  float camera_roll_speed = 0.f;
  float ship_rotate_to_camera_speed = 0.f;

  OrthonormalBasis ship_rotation_basis;
  OrthonormalBasis ship_velocity_basis;
} state;

inline float Lerpf(float a, float b, float alpha) {
  return a + (b - a) * alpha;
}

static void SetupFlightSimLevel(Tachyon* tachyon) {
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

static void CreateDebugMeshes(Tachyon* tachyon) {
  create(meshes.cube);
  create(meshes.cube);
  create(meshes.cube);
}

static void LoadTestShip(Tachyon* tachyon) {
  meshes.hull = Tachyon_AddMesh(tachyon, Tachyon_LoadMesh("./cosmodrone/assets/test-ship/hull.obj"), 1);
  meshes.streams = Tachyon_AddMesh(tachyon, Tachyon_LoadMesh("./cosmodrone/assets/test-ship/streams.obj"), 1);
  meshes.thrusters = Tachyon_AddMesh(tachyon, Tachyon_LoadMesh("./cosmodrone/assets/test-ship/thrusters.obj"), 1);
  meshes.trim = Tachyon_AddMesh(tachyon, Tachyon_LoadMesh("./cosmodrone/assets/test-ship/trim.obj"), 1);
}

static void UpdateViewDirections(Tachyon* tachyon, State& state) {
  auto& camera = tachyon->scene.camera;

  auto view_matrix = (
    camera.rotation.toMatrix4f() *
    tMat4f::translation(camera.position * tVec3f(-1.f))
  );

  state.view_forward_direction = tVec3f(
    view_matrix.m[8],
    view_matrix.m[9],
    view_matrix.m[10]
  ).invert();

  state.view_up_direction = tVec3f(
    view_matrix.m[4],
    view_matrix.m[5],
    view_matrix.m[6]
  );
}

const static auto FORWARD_VECTOR = tVec3f(0, 0, -1.f);
const static auto UP_VECTOR = tVec3f(0, 1.f, 0);
const static auto LEFT_VECTOR = tVec3f(-1.f, 0, 0);

static void IncreaseShipRotateToCameraSpeed(State& state, const float dt, const float factor) {
  state.ship_rotate_to_camera_speed += factor * dt;
}

static void HandleFlightControls(Tachyon* tachyon, State& state, const float dt) {
  // Handle forward movement
  if (is_key_held(tKey::W)) {
    state.ship_velocity += state.view_forward_direction * (1000.f * dt);

    IncreaseShipRotateToCameraSpeed(state, dt, 1.f);
  }

  // Handle roll
  if (is_key_held(tKey::Q)) {
    state.camera_roll_speed += dt;

    IncreaseShipRotateToCameraSpeed(state, dt, 5.f);
  } else if (is_key_held(tKey::E)) {
    state.camera_roll_speed -= dt;

    IncreaseShipRotateToCameraSpeed(state, dt, 5.f);
  }

  // Handle deceleration
  if (is_key_held(tKey::SHIFT)) {
    IncreaseShipRotateToCameraSpeed(state, dt, 1.f);
  }

  if (state.camera_roll_speed > 3.f) state.camera_roll_speed = 3.f;
  if (state.camera_roll_speed < -3.f) state.camera_roll_speed = -3.f;

  state.camera_roll_speed *= (1.f - dt);

  // Allow the ship to rotate to the camera orientation faster
  // the closer it is to the camera view's forward direction.
  // Rotate-to-camera speed values > 1 are reduced the more
  // the camera is pointed away from the ship direction.
  // This prevents rolling from being used as an exploit to
  // turn the ship around more quickly, since rolling ordinarily
  // rotates the ship faster to keep up with the camera
  // (necessary to reduce motion sickness).
  {
    auto& hull = objects(meshes.hull)[0];
    float rotate_speed_factor = tVec3f::dot(hull.rotation.getDirection(), state.view_forward_direction);

    if (rotate_speed_factor < 0.f) {
      rotate_speed_factor = 0.f;
    }

    rotate_speed_factor = powf(rotate_speed_factor, 20.f);

    if (state.ship_rotate_to_camera_speed > 1.f) {
      state.ship_rotate_to_camera_speed = Lerpf(state.ship_rotate_to_camera_speed, 1.f, 1.f - rotate_speed_factor);
    }
  }

  state.ship_rotate_to_camera_speed *= (1.f - dt);
  state.ship_position += state.ship_velocity * dt;
}

static void HandleFlightCamera(Tachyon* tachyon, State& state, const float dt) {
  if (is_window_focused()) {
    Quaternion turn = (
      Quaternion::fromAxisAngle(LEFT_VECTOR, -(float)tachyon->mouse_delta_y / 1000.f) *
      Quaternion::fromAxisAngle(UP_VECTOR, (float)tachyon->mouse_delta_x / 1000.f) *
      Quaternion::fromAxisAngle(FORWARD_VECTOR, state.camera_roll_speed * dt)
    );

    state.flight_camera_rotation = (turn * state.flight_camera_rotation).unit();
  }

  auto& camera = tachyon->scene.camera;

  camera.rotation = state.flight_camera_rotation;

  UpdateViewDirections(tachyon, state);

  camera.position = state.ship_position - state.view_forward_direction * 1000.f + state.view_up_direction * 150.f;
}

// @todo allow roll
// @todo move to engine
static Quaternion DirectionToQuaternion(const tVec3f& direction) {
  auto yaw = atan2f(state.ship_velocity.x, state.ship_velocity.z);
  float pitch = atan2f(state.ship_velocity.xz().magnitude(), state.ship_velocity.y) - 3.141592f / 2.f;

  return tOrientation(0.f, pitch, yaw).toQuaternion();
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

  if (state.ship_velocity.magnitude() > 0.f) {
    auto forward = state.ship_velocity.unit();
    auto up = UP_VECTOR;
    auto sideways = tVec3f::cross(forward, up);

    up = tVec3f::cross(sideways, forward);

    state.ship_velocity_basis.forward = forward;
    state.ship_velocity_basis.up = up;
    state.ship_velocity_basis.sideways = sideways;

    if (is_key_held(tKey::SHIFT)) {
      target_rotation = DirectionToQuaternion(state.ship_velocity_basis.forward.invert());
    }
  }

  // @todo will nlerp work here?
  auto rotation = Quaternion::slerp(hull.rotation, target_rotation, state.ship_rotate_to_camera_speed * dt);

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

static void UpdateShipDebugVectors(Tachyon* tachyon, State& state) {
  if (state.ship_velocity.magnitude() < 0.001f) {
    for (auto& cube : objects(meshes.cube)) {
      cube.scale = tVec3f(0.f);

      commit(cube);
    }

    return;
  }

  const static float thickness = 10.f;

  auto& forward = objects(meshes.cube)[0];
  auto& sideways = objects(meshes.cube)[1];
  auto& up = objects(meshes.cube)[2];

  forward.color = tVec3f(1.f, 0, 0);
  sideways.color = tVec3f(0, 1.f, 0);
  up.color = tVec3f(0, 0, 1.f);

  forward.position = state.ship_position + state.ship_velocity_basis.forward * 400.f;
  sideways.position = state.ship_position + state.ship_velocity_basis.sideways * 400.f;
  up.position = state.ship_position + state.ship_velocity_basis.up * 400.f;

  forward.rotation = DirectionToQuaternion(state.ship_velocity_basis.forward);
  sideways.rotation = DirectionToQuaternion(state.ship_velocity_basis.sideways);
  up.rotation = DirectionToQuaternion(state.ship_velocity_basis.up);

  forward.scale = tVec3f(thickness, thickness, 400.f);
  sideways.scale = tVec3f(400.f, thickness, thickness);
  up.scale = tVec3f(thickness, 400.f, thickness);

  commit(forward);
  commit(sideways);
  commit(up);
}

static void ShowDevLabels(Tachyon* tachyon, State& state) {
  auto& camera = tachyon->scene.camera;
  auto& hull = objects(meshes.hull)[0];

  add_dev_label("Ship position", hull.position.toString());
  add_dev_label("Ship velocity", state.ship_velocity.toString());
  add_dev_label("Camera position", camera.position.toString());
}

void Cosmodrone::StartGame(Tachyon* tachyon) {
  auto sunMesh = Tachyon_LoadMesh("./cosmodrone/assets/sun-sign.obj", tVec3f(-1.f, 1.f, 1.f));
  auto moonMesh = Tachyon_LoadMesh("./cosmodrone/assets/moon-sign.obj", tVec3f(-1.f, 1.f, 1.f));
  auto planeMesh = Tachyon_CreatePlaneMesh();
  auto sphereMesh = Tachyon_CreateSphereMesh(6);
  auto cubeMesh = Tachyon_CreateCubeMesh();

  meshes.sun = Tachyon_AddMesh(tachyon, sunMesh, 1);
  meshes.moon = Tachyon_AddMesh(tachyon, moonMesh, 1);
  meshes.plane = Tachyon_AddMesh(tachyon, planeMesh, 1);
  meshes.sphere = Tachyon_AddMesh(tachyon, sphereMesh, 40 * 40 * 40);
  meshes.cube = Tachyon_AddMesh(tachyon, cubeMesh, 3);

  LoadTestShip(tachyon);

  Tachyon_InitializeObjects(tachyon);

  SetupFlightSimLevel(tachyon);
  CreateDebugMeshes(tachyon);
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
  }

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

  HandleFlightControls(tachyon, state, dt);
  HandleFlightCamera(tachyon, state, dt);
  UpdateShip(tachyon, state, dt);

  // @todo dev mode only
  UpdateShipDebugVectors(tachyon, state);
  ShowDevLabels(tachyon, state);
}