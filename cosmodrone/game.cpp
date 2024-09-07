#include "cosmodrone/game.h"
#include "cosmodrone/world_setup.h"

using namespace Cosmodrone;

static MeshIds meshes;

static struct OrthonormalBasis {
  tVec3f forward = tVec3f(0, 0, -1.f);
  tVec3f up = tVec3f(0, 1.f, 0);
  tVec3f sideways = tVec3f(1.f, 0, 0);
};

enum FlightMode {
  MANUAL_CONTROL,
  AUTO_RETROGRADE
};

// @todo use within function scopes
static struct State {
  Quaternion target_camera_rotation = Quaternion(1.f, 0, 0, 0);
  FlightMode flight_mode = FlightMode::MANUAL_CONTROL;

  tVec3f view_forward_direction;
  tVec3f view_up_direction;

  tVec3f ship_position;
  tVec3f ship_velocity;
  float camera_roll_speed = 0.f;
  float ship_rotate_to_target_speed = 0.f;

  OrthonormalBasis ship_rotation_basis;
  OrthonormalBasis ship_velocity_basis;
} state;

// @todo move to engine
inline float Lerpf(float a, float b, float alpha) {
  return a + (b - a) * alpha;
}

// @todo move to engine
static Quaternion GetOppositeRotation(const Quaternion& rotation) {
  Quaternion opposite = rotation;

  opposite.x *= -1.f;
  opposite.y *= -1.f;
  opposite.z *= -1.f;

  return opposite;
}

// @todo allow roll
// @todo move to engine
static Quaternion DirectionToQuaternion(const tVec3f& direction) {
  auto yaw = atan2f(direction.x, direction.z);
  float pitch = atan2f(direction.xz().magnitude(), direction.y) - 3.141592f / 2.f;

  return tOrientation(0.f, pitch, yaw).toQuaternion();
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

static void HandleFlightControls(Tachyon* tachyon, State& state, const float dt) {
  // Handle forward thrust
  if (is_key_held(tKey::W)) {
    state.ship_velocity += state.ship_rotation_basis.forward * (1000.f * dt);
    state.ship_rotate_to_target_speed += dt;
    state.flight_mode = FlightMode::MANUAL_CONTROL;
  }

  // Handle roll maneuvers
  if (is_key_held(tKey::Q)) {
    state.camera_roll_speed += dt;
    state.ship_rotate_to_target_speed += 5.f * dt;
    state.flight_mode = FlightMode::MANUAL_CONTROL;
  } else if (is_key_held(tKey::E)) {
    state.camera_roll_speed -= dt;
    state.ship_rotate_to_target_speed += 5.f * dt;
    state.flight_mode = FlightMode::MANUAL_CONTROL;
  }

  // Handle auto-retrograde actions
  {
    if (did_press_key(tKey::SHIFT)) {
      state.flight_mode = FlightMode::AUTO_RETROGRADE;
    }

    // Allow the ship to swivel quickly in auto-retrograde mode
    if (state.flight_mode == FlightMode::AUTO_RETROGRADE) {
      state.ship_rotate_to_target_speed += 5.f * dt;
    }
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

    if (state.ship_rotate_to_target_speed > 1.f) {
      state.ship_rotate_to_target_speed = Lerpf(state.ship_rotate_to_target_speed, 1.f, 1.f - rotate_speed_factor);
    }
  }

  state.ship_rotate_to_target_speed *= (1.f - dt);
  state.ship_position += state.ship_velocity * dt;
}

static void HandleFlightCamera(Tachyon* tachyon, State& state, const float dt) {
  auto& camera = tachyon->scene.camera;
  float camera_lerp_speed_factor = 10.f;

  if (is_window_focused()) {
    Quaternion turn = (
      Quaternion::fromAxisAngle(LEFT_VECTOR, -(float)tachyon->mouse_delta_y / 1000.f) *
      Quaternion::fromAxisAngle(UP_VECTOR, (float)tachyon->mouse_delta_x / 1000.f) *
      Quaternion::fromAxisAngle(FORWARD_VECTOR, state.camera_roll_speed * dt)
    );

    state.target_camera_rotation = (turn * state.target_camera_rotation).unit();
  }

  if (state.flight_mode == FlightMode::AUTO_RETROGRADE) {
    Quaternion target_camera_rotation = GetOppositeRotation(objects(meshes.hull)[0].rotation);

    state.target_camera_rotation = Quaternion::slerp(state.target_camera_rotation, target_camera_rotation, dt);

    camera_lerp_speed_factor = 3.f;
  }

  float camera_lerp_alpha = camera_lerp_speed_factor * dt;
  if (camera_lerp_alpha > 1.f) camera_lerp_alpha = 1.f;

  camera.rotation = Quaternion::slerp(camera.rotation, state.target_camera_rotation, camera_lerp_alpha);

  UpdateViewDirections(tachyon, state);

  float ship_speed = state.ship_velocity.magnitude();
  float camera_radius = 1000.f + 300.f * (ship_speed / (ship_speed + 5000.f));

  camera.position = state.ship_position - state.view_forward_direction * camera_radius + state.view_up_direction * 150.f;
}

static void UpdateShip(Tachyon* tachyon, State& state, float dt) {
  auto& camera = tachyon->scene.camera;

  auto& hull = objects(meshes.hull)[0];
  auto& streams = objects(meshes.streams)[0];
  auto& thrusters = objects(meshes.thrusters)[0];
  auto& trim = objects(meshes.trim)[0];

  Quaternion target_ship_rotation = GetOppositeRotation(camera.rotation);

  if (state.ship_velocity.magnitude() > 0.f) {
    auto forward = state.ship_velocity.unit();
    auto up = UP_VECTOR;
    auto sideways = tVec3f::cross(forward, up).unit();

    up = tVec3f::cross(sideways, forward);

    state.ship_velocity_basis.forward = forward;
    state.ship_velocity_basis.up = up;
    state.ship_velocity_basis.sideways = sideways;
  }

  if (state.flight_mode == FlightMode::AUTO_RETROGRADE) {
    target_ship_rotation = DirectionToQuaternion(state.ship_velocity_basis.forward);
  }

  // @todo will nlerp work here?
  auto rotation = Quaternion::slerp(hull.rotation, target_ship_rotation, state.ship_rotate_to_target_speed * dt);

  hull.position = state.ship_position;
  streams.position = state.ship_position;
  thrusters.position = state.ship_position;
  trim.position = state.ship_position;

  hull.rotation = streams.rotation = thrusters.rotation = trim.rotation = rotation;

  commit(hull);
  commit(streams);
  commit(thrusters);
  commit(trim);

  state.ship_rotation_basis.forward = rotation.getDirection();
  state.ship_rotation_basis.up = rotation.getUpDirection();
  state.ship_rotation_basis.sideways = rotation.getLeftDirection().invert();
}

static void UpdateCelestialBodies(Tachyon* tachyon, State& state, float dt) {
  auto& camera = tachyon->scene.camera;

  auto& earth = objects(meshes.planet)[0];
  auto& moon = objects(meshes.planet)[1];
  auto& sun = objects(meshes.planet)[2];

  earth.position = camera.position + tVec3f(0, -5000000.f, 0);
  earth.color = tVec3f(0.3f, 0.7f, 1.f);
  earth.scale = tVec3f(1000000.f);
  earth.material = tVec4f(1.f, 0, 1.f, 0.1f);

  moon.position = camera.position + tVec3f(-2000000.f, -5000000.f, -5000000.f);
  moon.color = tVec3f(0.8f);
  moon.scale = tVec3f(250000.f);
  moon.material = tVec4f(1.f, 0, 0, 0.1f);

  sun.position = camera.position + tVec3f(5000000.f);
  sun.color = tVec4f(1.f, 0.7f, 0.5f, 1.f);
  sun.scale = tVec3f(350000.f);

  commit(earth);
  commit(moon);
  commit(sun);
}

// @todo move to debug.cpp
static void UpdateOrthonormalBasisDebugVectors(
  Tachyon* tachyon,
  State& state,
  tObject& forward,
  tObject& sideways,
  tObject& up,
  const OrthonormalBasis& basis,
  const float thickness,
  const float length
) {
  forward.position = state.ship_position + basis.forward * length;
  sideways.position = state.ship_position + basis.sideways * length;
  up.position = state.ship_position + basis.up * length;

  forward.rotation = DirectionToQuaternion(basis.forward);
  sideways.rotation = DirectionToQuaternion(basis.sideways);
  up.rotation = DirectionToQuaternion(basis.up);

  forward.scale = tVec3f(thickness, thickness, length);
  sideways.scale = tVec3f(thickness, thickness, length);
  up.scale = tVec3f(thickness, thickness, length);

  commit(forward);
  commit(sideways);
  commit(up);
}

// @todo move to debug.cpp
static void UpdateShipDebugVectors(Tachyon* tachyon, State& state) {
  {
    auto& forward = objects(meshes.cube)[0];
    auto& sideways = objects(meshes.cube)[1];
    auto& up = objects(meshes.cube)[2];

    forward.color = tVec4f(1.f, 0, 0, 1.f);
    sideways.color = tVec4f(0, 1.f, 0, 1.f);
    up.color = tVec4f(0, 0, 1.f, 1.f);

    UpdateOrthonormalBasisDebugVectors(tachyon, state, forward, sideways, up, state.ship_velocity_basis, 8.f, 400.f);
  }

  {
    auto& forward = objects(meshes.cube)[3];
    auto& sideways = objects(meshes.cube)[4];
    auto& up = objects(meshes.cube)[5];

    forward.color = tVec4f(1.f, 1.f, 0, 1.f);
    sideways.color = tVec4f(0, 1.f, 1.f, 1.f);
    up.color = tVec4f(1.f, 0, 1.f, 1.f);

    UpdateOrthonormalBasisDebugVectors(tachyon, state, forward, sideways, up, state.ship_rotation_basis, 6.f, 300.f);
  }
}

static void ShowDevLabels(Tachyon* tachyon, State& state) {
  auto& camera = tachyon->scene.camera;
  auto& hull = objects(meshes.hull)[0];

  add_dev_label("Ship position", state.ship_position.toString());
  add_dev_label("Ship velocity", state.ship_velocity.toString());
  add_dev_label("Camera position", camera.position.toString());
}

void Cosmodrone::StartGame(Tachyon* tachyon) {
  WorldSetup::LoadMeshes(tachyon, meshes);
  WorldSetup::InitializeGameWorld(tachyon, meshes);
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
  UpdateCelestialBodies(tachyon, state, dt);

  // auto& planet = objects(meshes.planet)[0];

  // planet.position = camera.position + tVec3f(0, -200000.f, 0);

  // commit(planet);

  // @todo dev mode only
  UpdateShipDebugVectors(tachyon, state);
  ShowDevLabels(tachyon, state);
}