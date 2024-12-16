#include "cosmodrone/autopilot.h"
#include "cosmodrone/flight_system.h"
#include "cosmodrone/game.h"
#include "cosmodrone/game_editor.h"
#include "cosmodrone/game_types.h"
#include "cosmodrone/hud_system.h"
#include "cosmodrone/mesh_library.h"
#include "cosmodrone/target_system.h"
#include "cosmodrone/world_behavior.h"
#include "cosmodrone/world_setup.h"

using namespace Cosmodrone;

// @todo move to constants
const static auto FORWARD_VECTOR = tVec3f(0, 0, -1.f);
const static auto UP_VECTOR = tVec3f(0, 1.f, 0);
const static auto RIGHT_VECTOR = tVec3f(1.f, 0, 0);

// @todo pass into StartGame() and RunGame() from main.cpp
static State state;

// @todo move to engine
static inline float Lerpf(float a, float b, float alpha) {
  return a + (b - a) * alpha;
}

// @todo move to engine
static inline tVec3f Lerpf(const tVec3f& a, const tVec3f& b, const float alpha) {
  return tVec3f(
    Lerpf(a.x, b.x, alpha),
    Lerpf(a.y, b.y, alpha),
    Lerpf(a.z, b.z, alpha)
  );
}

// @todo remove in favor of LookRotation()
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

// @todo move to Autopilot
static void AttemptDockingProcedure(State& state) {
  auto* tracker = TargetSystem::GetSelectedTargetTracker(state);

  if (tracker == nullptr) {
    return;
  }

  auto& target_object = tracker->object;
  auto& ship_position = state.ship_position;
  auto target_distance = (target_object.position - ship_position).magnitude();

  if (target_distance > 80000.f) {
    return;
  }

  state.flight_mode = FlightMode::AUTO_DOCK;
  state.auto_dock_stage = AutoDockStage::APPROACH_DECELERATION;
  state.docking_target = target_object;
}

const static float MAX_SHIP_SPEED = 15000.f;

static void HandleFlightControls(Tachyon* tachyon, State& state, const float dt) {
  bool is_issuing_control_action = false;

  // Handle forward thrust
  if (is_key_held(tKey::W)) {
    FlightSystem::ControlledThrustForward(state, dt);

    is_issuing_control_action = true;
  }

  // Enforce maximum ship speed
  float ship_speed = state.ship_velocity.magnitude();

  if (ship_speed > MAX_SHIP_SPEED) {
    state.ship_velocity = state.ship_velocity.unit() * MAX_SHIP_SPEED;
  }

  if (is_key_held(tKey::S)) {
    // Handle pitch up
    FlightSystem::ChangePitch(state, dt, 1.f);

    is_issuing_control_action = true;
  } else {
    // Reduce pitch gradually
    state.ship_pitch_factor *= (1.f - 5.f * dt);

    // Prevent a long reduction tail and just snap to 0
    if (abs(state.ship_pitch_factor) < 0.01f) {
      state.ship_pitch_factor = 0.f;
    }
  }

  // Handle yaw manuevers
  if (is_key_held(tKey::A)) {
    FlightSystem::YawLeft(state, dt);

    is_issuing_control_action = true;
  } else if (is_key_held(tKey::D)) {
    FlightSystem::YawRight(state, dt);

    is_issuing_control_action = true;
  }

  // Handle roll maneuvers
  if (is_key_held(tKey::Q)) {
    FlightSystem::RollLeft(state, dt);

    is_issuing_control_action = true;
  } else if (is_key_held(tKey::E)) {
    FlightSystem::RollRight(state, dt);

    is_issuing_control_action = true;
  }

  // Handle auto-prograde actions
  if (did_press_key(tKey::SHIFT)) {
    state.flight_mode = FlightMode::AUTO_PROGRADE;
    state.ship_rotate_to_target_speed = 0.f;
  }

  // Handle auto-retrograde actions
  if (did_press_key(tKey::SPACE)) {
    state.flight_mode = FlightMode::AUTO_RETROGRADE;
    state.ship_rotate_to_target_speed = 0.f;
  }

  // Handle auto-docking actions
  if (did_press_key(tKey::ENTER)) {
    AttemptDockingProcedure(state);
    state.ship_rotate_to_target_speed = 0.f;
  }

  // Allow the ship to swivel quickly in automatic flight modes
  if (
    state.flight_mode == FlightMode::AUTO_PROGRADE ||
    state.flight_mode == FlightMode::AUTO_RETROGRADE
  ) {
    state.ship_rotate_to_target_speed += 2.f * dt;
  }

  if (
    state.flight_mode == FlightMode::AUTO_DOCK &&
    state.auto_dock_stage < AutoDockStage::APPROACH
  ) {
    state.ship_rotate_to_target_speed += 1.f * dt;
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
    float forward_alignment = tVec3f::dot(state.ship_rotation_basis.forward, state.view_forward_direction);
    if (forward_alignment < 0.f) forward_alignment = 0.f;

    if (state.ship_pitch_factor == 0.f) {
      // Only do exponential tapering when not pitching.
      // When pitching, we want to allow the rotate-to-target
      // value to be mostly preserved.
      forward_alignment = powf(forward_alignment, 20.f);

      if (state.ship_rotate_to_target_speed > 1.f) {
        state.ship_rotate_to_target_speed = Lerpf(state.ship_rotate_to_target_speed, 1.f, 1.f - forward_alignment);
      }
    }
  }

  state.ship_rotate_to_target_speed *= (1.f - dt);
  state.ship_position += state.ship_velocity * dt;

  // When panning the camera around while not issuing ship controls,
  // rapidly slow the ship's natural rotation drift
  if (
    state.flight_mode == FlightMode::MANUAL_CONTROL &&
    !is_issuing_control_action &&
    (tachyon->mouse_delta_x != 0 || tachyon->mouse_delta_y != 0)
  ) {
    state.ship_rotate_to_target_speed *= (1.f - 5.f * dt);
  }
}

static void HandleFlightCamera(Tachyon* tachyon, State& state, const float dt) {
  auto& camera = tachyon->scene.camera;
  auto& meshes = state.meshes;
  float camera_lerp_speed_factor = 10.f;

  // Clamp roll speed
  if (state.camera_roll_speed > 3.f) state.camera_roll_speed = 3.f;
  if (state.camera_roll_speed < -3.f) state.camera_roll_speed = -3.f;

  state.camera_roll_speed *= (1.f - dt);

  if (is_window_focused()) {
    Quaternion turn = (
      Quaternion::fromAxisAngle(RIGHT_VECTOR, (float)tachyon->mouse_delta_y / 1000.f) *
      Quaternion::fromAxisAngle(UP_VECTOR, (float)tachyon->mouse_delta_x / 1000.f) *
      Quaternion::fromAxisAngle(FORWARD_VECTOR, state.camera_roll_speed * dt)
    );

    state.target_camera_rotation *= turn;

    // Swiveling
    {
      if (is_key_held(tKey::A)) {
        state.target_camera_rotation = state.target_camera_rotation * Quaternion::fromAxisAngle(state.ship_rotation_basis.up, -1.f * dt);
      } else if (is_key_held(tKey::D)) {
        state.target_camera_rotation = state.target_camera_rotation * Quaternion::fromAxisAngle(state.ship_rotation_basis.up, 1.f * dt);
      }
    }

    // Zoom in/out
    {
      if (did_wheel_down()) {
        state.ship_camera_distance_target += 1000.f;
      } else if (did_wheel_up()) {
        state.ship_camera_distance_target -= 1000.f;
        if (state.ship_camera_distance_target < 1800.f) state.ship_camera_distance_target = 1800.f;
      }
    }

    state.target_camera_rotation = state.target_camera_rotation.unit();
  }

  if (
    state.flight_mode == FlightMode::AUTO_PROGRADE ||
    state.flight_mode == FlightMode::AUTO_RETROGRADE || (
      state.flight_mode == FlightMode::AUTO_DOCK &&
      state.auto_dock_stage == AutoDockStage::APPROACH_ALIGNMENT &&
      Autopilot::GetDockingAlignment(state, Autopilot::GetDockingPosition(tachyon, state)) > 0.5f
    )
  ) {
    // Gradually move the camera behind the player ship
    state.target_camera_rotation = Quaternion::slerp(
      state.target_camera_rotation,
      objects(meshes.hull)[0].rotation.opposite(),
      2.f * dt
    );
  }

  if (state.ship_pitch_factor != 0.f) {
    auto new_target =
      Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), -20.f * state.ship_pitch_factor * dt) *
      objects(meshes.hull)[0].rotation.opposite();

    state.target_camera_rotation = Quaternion::slerp(
      state.target_camera_rotation,
      new_target,
      2.f * abs(state.ship_pitch_factor) * dt
    );
  }

  float camera_lerp_alpha = camera_lerp_speed_factor * dt;
  if (camera_lerp_alpha > 1.f) camera_lerp_alpha = 1.f;

  camera.rotation = Quaternion::slerp(camera.rotation, state.target_camera_rotation, camera_lerp_alpha);

  UpdateViewDirections(tachyon, state);

  state.ship_camera_distance = Lerpf(state.ship_camera_distance, state.ship_camera_distance_target, 5.f * dt);

  float ship_speed = state.ship_velocity.magnitude();
  float speed_zoom_ratio = ship_speed / (ship_speed + 5000.f);
  float camera_radius = state.ship_camera_distance + 250.f * speed_zoom_ratio;

  camera.fov = 45.f + 10.f * speed_zoom_ratio;
  camera.position = state.ship_position - state.view_forward_direction * camera_radius + state.view_up_direction * 500.f;
}

// @todo move to HUDSystem
static void HandleFlightArrows(Tachyon* tachyon, State& state, const float dt) {
  const static float MIN_SPAWN_DISTANCE = 5000.f;
  const static float MAX_SPAWN_DISTANCE = 20000.f;

  auto& meshes = state.meshes;
  float speed = state.ship_velocity.magnitude();

  if (speed == 0.f) {
    return;
  }

  // Reduce distance to next flight path node spawn
  state.flight_path_spawn_distance_remaining -= speed * dt;

  // Move incoming flight path nodes toward the ship
  auto& flight_path = state.incoming_flight_path;
  tVec3f sideways = tVec3f::cross(state.ship_velocity_basis.forward, state.ship_rotation_basis.up).unit();
  tVec3f bottom_offset = state.ship_rotation_basis.up * -750.f;
  tVec3f left_offset = sideways * -750.f;
  tVec3f right_offset = sideways * 750.f;

  // Reset arrows each frame
  for (auto& arrow : objects(meshes.hud_flight_arrow)) {
    arrow.scale = 0.f;
    arrow.rotation = Quaternion::FromDirection(state.ship_velocity_basis.forward, state.ship_rotation_basis.up);

    commit(arrow);
  }

  float speed_ratio = state.ship_velocity.magnitude() / MAX_SHIP_SPEED;

  // Recalculate/reposition visible arrows
  for (int32 i = flight_path.size() - 1; i >= 0; i--) {
    auto& node = flight_path[i];
    auto direction_to_ship = (state.ship_position - node.position).unit();

    node.distance -= speed * dt;

    // Gradually curve nodes onto the updated flight path as it changes in real-time.
    // Determine the node's "progress" toward the ship, and blend between its original
    // spawn position and a position directly forward along the ship's trajectory.
    float target_distance = Lerpf(MAX_SPAWN_DISTANCE, node.spawn_distance, speed_ratio);
    float alpha = 1.f - node.distance / target_distance;
    tVec3f velocity_position = state.ship_position + state.ship_velocity_basis.forward * node.distance;

    node.position = Lerpf(node.spawn_position, velocity_position, alpha);

    float velocity_alignment = tVec3f::dot(direction_to_ship, state.ship_velocity_basis.forward);
    float forward_alignment = tVec3f::dot(direction_to_ship, state.ship_rotation_basis.forward);

    if (
      node.distance <= 0.f ||
      velocity_alignment > 0.f ||
      forward_alignment > 0.f
    ) {
      flight_path.erase(flight_path.begin() + i);

      continue;
    }

    float path_progress = 1.f - node.distance / node.spawn_distance;
    float brightness = 1.f;
    if (path_progress < 0.1f) brightness = path_progress / 0.1f;
    if (path_progress > 0.8f) brightness = 1.f - (path_progress - 0.8f) / 0.2f;

    if (forward_alignment > -0.1f) {
      brightness *= (forward_alignment * -10.f);
    }

    auto& arrow_1 = objects(meshes.hud_flight_arrow)[i * 2];
    auto& arrow_2 = objects(meshes.hud_flight_arrow)[i * 2 + 1];

    arrow_1.position = node.position + bottom_offset + left_offset;
    arrow_2.position = node.position + bottom_offset + right_offset;

    arrow_1.scale = arrow_2.scale = 200.f;
    // @todo change color in auto-docking approach mode
    arrow_1.color = arrow_2.color = tVec4f(0.1f, 0.2f, 1.f, brightness);

    commit(arrow_1);
    commit(arrow_2);
  }

  // Add new flight path nodes
  if (state.flight_path_spawn_distance_remaining <= 0.f) {
    FlightPathNode node;

    node.spawn_distance = Lerpf(MIN_SPAWN_DISTANCE, MAX_SPAWN_DISTANCE, speed_ratio);
    node.position = state.ship_position + state.ship_velocity_basis.forward * node.spawn_distance;
    node.spawn_position = node.position;
    node.distance = node.spawn_distance;

    flight_path.push_back(node);

    state.flight_path_spawn_distance_remaining = node.spawn_distance * 0.25f;

    state.flight_arrow_cycle_step++;
    if (state.flight_arrow_cycle_step > 4) state.flight_arrow_cycle_step = 0;
  }
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

static void HandlePlayerDrone(Tachyon* tachyon, State& state, const float dt) {
  auto& camera = tachyon->scene.camera;
  auto& meshes = state.meshes;

  auto& hull = objects(meshes.hull)[0];
  auto& streams = objects(meshes.streams)[0];
  auto& thrusters = objects(meshes.thrusters)[0];
  auto& trim = objects(meshes.trim)[0];

  if (state.ship_velocity.magnitude() > 0.f) {
    UpdateShipVelocityBasis(state);
  }

  if (!Autopilot::IsAutopilotActive(state)) {
    state.target_ship_rotation = camera.rotation.opposite();

    if (state.ship_pitch_factor != 0.f) {
      float pitch_change = 50.f * state.ship_pitch_factor * dt;

      state.target_ship_rotation =
        state.target_ship_rotation *
        Quaternion::fromAxisAngle(tVec3f(1.f, 0, 0), pitch_change);

      FlightSystem::HandlePitch(state, dt);
    }
  }

  // @todo will nlerp work here?
  auto rotation = Quaternion::slerp(hull.rotation, state.target_ship_rotation, state.ship_rotate_to_target_speed * dt);

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

  // Velocity
  {
    auto& forward = objects(meshes.cube)[0];
    auto& sideways = objects(meshes.cube)[1];
    auto& up = objects(meshes.cube)[2];

    forward.color = tVec4f(1.f, 0, 0, 1.f);
    sideways.color = tVec4f(0, 1.f, 0, 1.f);
    up.color = tVec4f(0, 0, 1.f, 1.f);

    UpdateOrthonormalBasisDebugVectors(tachyon, state, forward, sideways, up, state.ship_velocity_basis, 15.f, 500.f);
  }

  // Rotation
  {
    auto& forward = objects(meshes.cube)[3];
    auto& sideways = objects(meshes.cube)[4];
    auto& up = objects(meshes.cube)[5];

    forward.color = tVec4f(1.f, 1.f, 0, 1.f);
    sideways.color = tVec4f(0, 1.f, 1.f, 1.f);
    up.color = tVec4f(1.f, 0, 1.f, 1.f);

    UpdateOrthonormalBasisDebugVectors(tachyon, state, forward, sideways, up, state.ship_rotation_basis, 12.f, 400.f);
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

  // @todo UI::Initialize()
  {
    state.ui.target_indicator = Tachyon_CreateUIElement("./cosmodrone/assets/ui/target.png");
    state.ui.zone_target_indicator = Tachyon_CreateUIElement("./cosmodrone/assets/ui/zone-target.png");
    state.ui.selected_target_corner = Tachyon_CreateUIElement("./cosmodrone/assets/ui/selected-target-corner.png");
    state.ui.selected_target_center = Tachyon_CreateUIElement("./cosmodrone/assets/ui/selected-target-center.png");

    state.ui.cascadia_mono_26 = Tachyon_CreateUIText("./fonts/CascadiaMonoNF.ttf", 26);
    state.ui.cascadia_mono_32 = Tachyon_CreateUIText("./fonts/CascadiaMonoNF.ttf", 32);
  }
}

void Cosmodrone::UpdateGame(Tachyon* tachyon, const float dt) {
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
  objects(meshes.hud_flight_arrow).disabled = state.is_editor_active;
  objects(meshes.hud_wedge).disabled = state.is_editor_active;
  objects(meshes.antenna_3_wireframe).disabled = state.is_editor_active;

  // @todo dev mode only
  if (state.is_editor_active) {
    Editor::HandleEditor(tachyon, state, dt);

    return;
  } else {
    objects(state.meshes.editor_guideline).disabled = true;
  }

  HandleFlightControls(tachyon, state, dt);

  Autopilot::HandleAutopilot(tachyon, state, dt);

  HandleFlightCamera(tachyon, state, dt);
  HandleFlightArrows(tachyon, state, dt);
  HandlePlayerDrone(tachyon, state, dt);

  HUDSystem::HandleHUD(tachyon, state, dt);
  TargetSystem::HandleTargetTrackers(tachyon, state, dt);
  WorldBehavior::UpdateWorld(tachyon, state, dt);

  // @todo dev mode only
  if (tachyon->show_developer_tools) {
    UpdateShipDebugVectors(tachyon, state);
    ShowDevLabels(tachyon, state);
  }

  // @todo factor
  {
    auto s = Tachyon_GetMicroseconds();

    // @todo process one mesh per frame
    Tachyon_UseLodByDistance(tachyon, meshes.girder_1, 100000.f);
    Tachyon_UseLodByDistance(tachyon, meshes.girder_2, 80000.f);
    Tachyon_UseLodByDistance(tachyon, meshes.girder_3, 80000.f);
    Tachyon_UseLodByDistance(tachyon, meshes.grate_1, 50000.f);

    Tachyon_UseLodByDistance(tachyon, meshes.girder_4_frame, 100000.f);
    Tachyon_UseLodByDistance(tachyon, meshes.silo_3_body, 100000.f);
    Tachyon_UseLodByDistance(tachyon, meshes.silo_3_frame, 100000.f);

    auto t = Tachyon_GetMicroseconds() - s;

    // @todo dev mode only
    add_dev_label("HandleLevelsOfDetail", std::to_string(t) + "us");
  }
}