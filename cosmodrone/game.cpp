#include "cosmodrone/game.h"
#include "cosmodrone/game_editor.h"
#include "cosmodrone/game_types.h"
#include "cosmodrone/mesh_library.h"
#include "cosmodrone/world_behavior.h"
#include "cosmodrone/world_setup.h"

using namespace Cosmodrone;

const static auto FORWARD_VECTOR = tVec3f(0, 0, -1.f);
const static auto UP_VECTOR = tVec3f(0, 1.f, 0);
const static auto RIGHT_VECTOR = tVec3f(1.f, 0, 0);

// @todo pass into StartGame() and RunGame() from main.cpp
static State state;

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
  auto pitch = atan2f(direction.xz().magnitude(), direction.y) - t_HALF_PI;

  return (
    Quaternion::fromAxisAngle(UP_VECTOR, yaw) *
    Quaternion::fromAxisAngle(RIGHT_VECTOR, pitch)
  );
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

static void HandleFlightControls(Tachyon* tachyon, State& state, const float dt) {
  // Handle forward thrust
  if (is_key_held(tKey::W)) {
    state.ship_velocity -= state.ship_velocity_basis.forward * 800.f * dt;
    state.ship_velocity += state.ship_rotation_basis.forward * 1500.f * dt;

    state.ship_rotate_to_target_speed += 5.f * dt;
    state.flight_mode = FlightMode::MANUAL_CONTROL;
  }

  // Enforce maximum ship speed
  const static float MAX_SHIP_SPEED = 15000.f;
  float ship_speed = state.ship_velocity.magnitude();

  if (ship_speed > MAX_SHIP_SPEED) {
    state.ship_velocity = state.ship_velocity.unit() * MAX_SHIP_SPEED;
  }

  // Handle yaw manuevers
  if (is_key_held(tKey::A)) {
    state.ship_rotate_to_target_speed += 5.f * dt;
    state.flight_mode = FlightMode::MANUAL_CONTROL;
  } else if (is_key_held(tKey::D)) {
    state.ship_rotate_to_target_speed += 5.f * dt;
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

  if (state.camera_roll_speed > 3.f) state.camera_roll_speed = 3.f;
  if (state.camera_roll_speed < -3.f) state.camera_roll_speed = -3.f;

  state.camera_roll_speed *= (1.f - dt);

  // Handle auto-prograde actions
  if (did_press_key(tKey::SHIFT)) {
    state.flight_mode = FlightMode::AUTO_PROGRADE;
  }

  // Handle auto-retrograde actions
  if (did_press_key(tKey::SPACE)) {
    state.flight_mode = FlightMode::AUTO_RETROGRADE;
  }

  // Allow the ship to swivel quickly in automatic flight modes
  if (
    state.flight_mode == FlightMode::AUTO_PROGRADE ||
    state.flight_mode == FlightMode::AUTO_RETROGRADE
  ) {
    state.ship_rotate_to_target_speed += 5.f * dt;
  }

  // Allow the ship to rotate to the camera orientation faster
  // the closer it is to the camera view's forward direction.
  // Rotate-to-camera speed values > 1 are reduced the more
  // the camera is pointed away from the ship direction.
  // This prevents rolling from being used as an exploit to
  // turn the ship around more quickly, since rolling ordinarily
  // rotates the ship faster to keep up with the camera
  // (necessary to reduce motion sickness).
  {
    auto& meshes = state.meshes;
    auto& hull = objects(meshes.hull)[0];
    float rotate_speed_factor = tVec3f::dot(hull.rotation.getDirection(), state.view_forward_direction);

    if (rotate_speed_factor < 0.f) rotate_speed_factor = 0.f;

    rotate_speed_factor = powf(rotate_speed_factor, 20.f);

    if (state.ship_rotate_to_target_speed > 1.f) {
      state.ship_rotate_to_target_speed = Lerpf(state.ship_rotate_to_target_speed, 1.f, 1.f - rotate_speed_factor);
    }
  }

  state.ship_rotate_to_target_speed *= (1.f - dt);
  state.ship_position += state.ship_velocity * dt;
}

static void HandleAutopilot(Tachyon* tachyon, State& state, const float dt) {
  switch (state.flight_mode) {
    case FlightMode::AUTO_RETROGRADE: {
      // Figure out how 'backward' the ship is pointed
      float reverse_dot = tVec3f::dot(state.ship_rotation_basis.forward, state.ship_velocity.unit());

      if (reverse_dot < -0.f) {
        // Use the current speed to determine how much we need to accelerate in the opposite direction
        float acceleration = state.ship_velocity.magnitude() / 2.f;
        if (acceleration > 5000.f) acceleration = 5000.f;
        if (acceleration < 500.f) acceleration = 500.f;
        // Increase acceleration the more the ship is aligned with the 'backward' vector
        float speed = acceleration * powf(-reverse_dot, 15.f);

        state.ship_velocity += state.ship_rotation_basis.forward * speed * dt;
      }

      if (state.ship_velocity.magnitude() < 25.f) {
        state.flight_mode = FlightMode::MANUAL_CONTROL;
      }

      break;
    }
  }
}

static void HandleFlightCamera(Tachyon* tachyon, State& state, const float dt) {
  const static float BASE_SHIP_DISTANCE = 1000.f;

  auto& camera = tachyon->scene.camera;
  auto& meshes = state.meshes;
  float camera_lerp_speed_factor = 10.f;

  if (is_window_focused()) {
    Quaternion turn = (
      Quaternion::fromAxisAngle(RIGHT_VECTOR, (float)tachyon->mouse_delta_y / 1000.f) *
      Quaternion::fromAxisAngle(UP_VECTOR, (float)tachyon->mouse_delta_x / 1000.f) *
      Quaternion::fromAxisAngle(FORWARD_VECTOR, state.camera_roll_speed * dt)
    );

    state.target_camera_rotation *= turn;

    if (is_key_held(tKey::A)) {
      state.target_camera_rotation = state.target_camera_rotation * Quaternion::fromAxisAngle(state.ship_rotation_basis.up, -0.5f * dt);
    } else if (is_key_held(tKey::D)) {
      state.target_camera_rotation = state.target_camera_rotation * Quaternion::fromAxisAngle(state.ship_rotation_basis.up, 0.5f * dt);
    }

    state.target_camera_rotation = state.target_camera_rotation.unit();
  }

  if (state.flight_mode == FlightMode::AUTO_PROGRADE || state.flight_mode == FlightMode::AUTO_RETROGRADE) {
    state.target_camera_rotation = GetOppositeRotation(objects(meshes.hull)[0].rotation);

    camera_lerp_speed_factor = 3.f;
  }

  float camera_lerp_alpha = camera_lerp_speed_factor * dt;
  if (camera_lerp_alpha > 1.f) camera_lerp_alpha = 1.f;

  camera.rotation = Quaternion::slerp(camera.rotation, state.target_camera_rotation, camera_lerp_alpha);

  UpdateViewDirections(tachyon, state);

  float ship_speed = state.ship_velocity.magnitude();
  float camera_radius = BASE_SHIP_DISTANCE + 500.f * (ship_speed / (ship_speed + 5000.f));

  camera.position = state.ship_position - state.view_forward_direction * camera_radius + state.view_up_direction * 150.f;
}

static void UpdateShipVelocityBasis(State& state) {
  auto forward = state.ship_velocity.unit();
  auto up = UP_VECTOR;
  auto sideways = tVec3f::cross(forward, up).unit();

  up = tVec3f::cross(sideways, forward);

  state.ship_velocity_basis.forward = forward;
  state.ship_velocity_basis.up = up;
  state.ship_velocity_basis.sideways = sideways;
}

static void UpdateShip(Tachyon* tachyon, State& state, float dt) {
  auto& camera = tachyon->scene.camera;
  auto& meshes = state.meshes;

  auto& hull = objects(meshes.hull)[0];
  auto& streams = objects(meshes.streams)[0];
  auto& thrusters = objects(meshes.thrusters)[0];
  auto& trim = objects(meshes.trim)[0];

  // @todo fix ship model orientation
  auto target_ship_rotation = GetOppositeRotation(camera.rotation);

  if (state.ship_velocity.magnitude() > 0.f) {
    UpdateShipVelocityBasis(state);
  }

  // @todo move to HandleAutopilot()
  if (state.flight_mode == FlightMode::AUTO_PROGRADE) {
    target_ship_rotation = DirectionToQuaternion(state.ship_velocity_basis.forward.invert());
  }

  // @todo move to HandleAutopilot()
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
  auto& meshes = state.meshes;

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
  auto& meshes = state.meshes;
  auto& hull = objects(meshes.hull)[0];

  add_dev_label("Game time", std::to_string(state.current_game_time));
  add_dev_label("Ship position", state.ship_position.toString());
  add_dev_label("Ship velocity", state.ship_velocity.toString());
  add_dev_label("Ship speed", std::to_string(state.ship_velocity.magnitude()));
  add_dev_label("Camera position", camera.position.toString());
  add_dev_label("Camera rotation", camera.rotation.toString());
}

void Cosmodrone::StartGame(Tachyon* tachyon) {
  MeshLibrary::LoadMeshes(tachyon, state);
  WorldSetup::InitializeGameWorld(tachyon, state);
  Editor::InitializeEditor(tachyon, state);
}

void Cosmodrone::RunGame(Tachyon* tachyon, const float dt) {
  auto& camera = tachyon->scene.camera;
  auto& meshes = state.meshes;

  tachyon->scene.transform_origin = camera.position;

  // @todo dev mode only
  if (did_press_key(tKey::NUM_1)) {
    if (state.is_editor_active) {
      Editor::DisableEditor(tachyon, state);
    } else {
      Editor::EnableEditor(tachyon, state);
    }
  }

  // @todo dev mode only
  objects(meshes.cube).disabled = state.is_editor_active || !tachyon->show_developer_tools;

  // @todo dev mode only
  if (state.is_editor_active) {
    Editor::HandleEditor(tachyon, state, dt);

    return;
  }

  // @todo dev mode only
  objects(state.meshes.editor_guideline).disabled = true;

  HandleFlightControls(tachyon, state, dt);
  HandleAutopilot(tachyon, state, dt);
  HandleFlightCamera(tachyon, state, dt);
  UpdateShip(tachyon, state, dt);

  WorldBehavior::UpdateWorld(tachyon, state, dt);

  // @todo dev mode only
  if (tachyon->show_developer_tools) {
    UpdateShipDebugVectors(tachyon, state);
    ShowDevLabels(tachyon, state);
  }
}