#include "metro/editor_utilities.h"

using namespace metro;

tVec3f EditorUtilities::GetClosestBasisAxis(const Quaternion& rotation, const tVec3f& vector) {
  tVec3f basis_up = rotation.getUpDirection();
  tVec3f basis_right = rotation.getLeftDirection().invert();
  tVec3f basis_forward = rotation.getDirection();

  float dot_up = tVec3f::dot(vector, basis_up);
  float dot_right = tVec3f::dot(vector, basis_right);
  float dot_forward = tVec3f::dot(vector, basis_forward);

  float up_factor = abs(dot_up);
  float right_factor = abs(dot_right);
  float forward_factor = abs(dot_forward);

  if (up_factor > right_factor && up_factor > forward_factor) {
    return dot_up < 0.f ? basis_up.invert() : basis_up;
  } else if (right_factor > up_factor && right_factor > forward_factor) {
    return dot_right < 0.f ? basis_right.invert() : basis_right;
  } else {
    return dot_forward < 0.f ? basis_forward.invert() : basis_forward;
  }
}

tVec3f EditorUtilities::GetClosestWorldAxis(const tVec3f& vector) {
  float abs_x = abs(vector.x);
  float abs_y = abs(vector.y);
  float abs_z = abs(vector.z);

  if (abs_x > abs_y && abs_x > abs_z) {
    return tVec3f(vector.x, 0, 0).unit();
  } else if (abs_y > abs_x && abs_y > abs_z) {
    return tVec3f(0, vector.y, 0).unit();
  } else {
    return tVec3f(0, 0, vector.z).unit();
  }
}

tVec3f EditorUtilities::GetScalingAxis(const tVec3f& basis_axis, const Quaternion& basis_rotation) {
  tVec3f scaling_axis;

  // Transform the basis axis into a world axis which we can use
  // as a cartesian axis for scaling along x, y, or z.
  scaling_axis = basis_rotation.toMatrix4f().inverse() * basis_axis;
  scaling_axis = GetClosestWorldAxis(scaling_axis);

  // Restrict to positive axis directions so that mouse movements always
  // scale up or down in the expected fashion, regardless of orientation
  if (scaling_axis.x == -1.f) scaling_axis.x = 1.f;
  if (scaling_axis.z == -1.f) scaling_axis.z = 1.f;
  if (scaling_axis.y == -1.f) scaling_axis.y = 1.f;

  return scaling_axis;
}

void EditorUtilities::ShowPositionGizmo(Tachyon* tachyon, const tVec3f& position, const Quaternion& basis_rotation) {
  const float line_thickness = 15.f;
  const tVec3f cone_scale = tVec3f(40.f);

  tMat4f basis_matrix = basis_rotation.toMatrix4f();

  // Left vector
  {
    tVec3f vector = basis_matrix * tVec3f(1.f, 0, 0);

    DebugLineConfig left_line = {
      .position = position,
      .vector = vector * 250.f,
      .color = tVec3f(1.f, 0, 0),
      .thickness = line_thickness
    };

    DebugShapeConfig left_cone = {
      .position = left_line.position + left_line.vector,
      .scale = cone_scale,
      .direction = vector,
      .color = tVec3f(1.f, 0, 0)
    };

    Debug::ShowDebugLine(tachyon, left_line);
    Debug::ShowDebugCone(tachyon, left_cone);
  }

  // Up vector
  {
    tVec3f vector = basis_matrix * tVec3f(0, 1.f, 0);

    DebugLineConfig up_line = {
      .position = position,
      .vector = vector * 250.f,
      .color = tVec3f(0, 1.f, 0),
      .thickness = line_thickness
    };

    DebugShapeConfig up_cone = {
      .position = up_line.position + up_line.vector,
      .scale = cone_scale,
      .direction = vector,
      .color = tVec3f(0, 1.f, 0)
    };

    Debug::ShowDebugLine(tachyon, up_line);
    Debug::ShowDebugCone(tachyon, up_cone);
  }

  // Forward vector
  {
    tVec3f vector = basis_matrix * tVec3f(0, 0, 1.f);

    DebugLineConfig forward_line = {
      .position = position,
      .vector = vector * 250.f,
      .color = tVec3f(0, 0, 1.f),
      .thickness = line_thickness
    };

    DebugShapeConfig forward_cone = {
      .position = forward_line.position + forward_line.vector,
      .scale = cone_scale,
      .direction = vector,
      .color = tVec3f(0, 0, 1.f)
    };

    Debug::ShowDebugLine(tachyon, forward_line);
    Debug::ShowDebugCone(tachyon, forward_cone);
  }
}

void EditorUtilities::ShowScaleGizmo(Tachyon* tachyon, const tVec3f& position, const Quaternion& basis_rotation, bool restricted) {
  const float line_thickness = 15.f;
  const tVec3f tip_scale = tVec3f(15.f);

  tMat4f basis_matrix = basis_rotation.toMatrix4f();

  // Left vector
  {
    tVec3f vector = basis_matrix * tVec3f(1.f, 0, 0);
    tVec3f color = restricted ? tVec3f(1.f, 1.f, 0) : tVec3f(1.f, 0, 0);

    DebugLineConfig left_line = {
      .position = position,
      .vector = vector * 250.f,
      .color = color,
      .thickness = line_thickness
    };

    DebugCubeConfig left_tip = {
      .position = left_line.position + left_line.vector,
      .scale = tip_scale,
      .rotation = basis_rotation,
      .color = color
    };

    Debug::ShowDebugLine(tachyon, left_line);
    Debug::ShowDebugCube(tachyon, left_tip);
  }

  // Up vector
  {
    tVec3f vector = basis_matrix * tVec3f(0, 1.f, 0);
    tVec3f color = restricted ? tVec3f(1.f, 1.f, 0) : tVec3f(0, 1.f, 0);

    DebugLineConfig up_line = {
      .position = position,
      .vector = vector * 250.f,
      .color = color,
      .thickness = line_thickness
    };

    DebugCubeConfig up_tip = {
      .position = up_line.position + up_line.vector,
      .scale = tip_scale,
      .rotation = basis_rotation,
      .color = color
    };

    Debug::ShowDebugLine(tachyon, up_line);
    Debug::ShowDebugCube(tachyon, up_tip);
  }

  // Forward vector
  {
    tVec3f vector = basis_matrix * tVec3f(0, 0, 1.f);
    tVec3f color = restricted ? tVec3f(1.f, 1.f, 0) : tVec3f(0, 0, 1.f);

    DebugLineConfig forward_line = {
      .position = position,
      .vector = vector * 250.f,
      .color = color,
      .thickness = line_thickness
    };

    DebugCubeConfig forward_tip = {
      .position = forward_line.position + forward_line.vector,
      .scale = tip_scale,
      .rotation = basis_rotation,
      .color = color
    };

    Debug::ShowDebugLine(tachyon, forward_line);
    Debug::ShowDebugCube(tachyon, forward_tip);
  }
}

void EditorUtilities::ShowRotationGizmo(Tachyon* tachyon, const tVec3f& position, const Quaternion& basis_rotation, bool restricted) {
  const tVec3f ring_scale = tVec3f(300.f);

  tMat4f basis_matrix = basis_rotation.toMatrix4f();

  // X axis ring
  {
    tVec3f vector = basis_matrix * tVec3f(1.f, 0, 0);

    DebugShapeConfig x_ring = {
      .position = position,
      .scale = ring_scale,
      .direction = vector,
      .color = restricted ? tVec3f(0.4f) : tVec3f(1.f, 0, 0)
    };

    Debug::ShowDebugRing(tachyon, x_ring);
  }

  // Y axis ring
  {
    tVec3f vector = basis_matrix * tVec3f(0, 1.f, 0);

    DebugShapeConfig y_ring = {
      .position = position,
      .scale = ring_scale,
      .direction = vector,
      .color = tVec3f(0, 1.f, 0)
    };

    Debug::ShowDebugRing(tachyon, y_ring);
  }

  // Z axis ring
  {
    tVec3f vector = basis_matrix * tVec3f(0, 0, 1.f);

    DebugShapeConfig z_ring = {
      .position = position,
      .scale = ring_scale,
      .direction = vector,
      .color = restricted ? tVec3f(0.4f) : tVec3f(0, 0, 1.f)
    };

    Debug::ShowDebugRing(tachyon, z_ring);
  }
}

void EditorUtilities::UseWASDCameraMovement(Tachyon* tachyon, State& state, const float speed) {
  auto& camera = tachyon->scene.camera;
  tVec3f camera_forward = camera.orientation.getDirection();
  tVec3f camera_left = camera.orientation.getLeftDirection();

  if (is_key_held(tKey::W)) {
    camera.position += camera_forward * speed * state.dt;
  }

  if (is_key_held(tKey::A)) {
    camera.position += camera_left * speed * state.dt;
  }

  if (is_key_held(tKey::S)) {
    camera.position -= camera_forward * speed * state.dt;
  }

  if (is_key_held(tKey::D)) {
    camera.position -= camera_left * speed * state.dt;
  }
}

void EditorUtilities::UseMouseCameraLookaround(Tachyon* tachyon, State& state, const float sensitivity) {
  auto& camera = tachyon->scene.camera;

  camera.orientation.yaw += tachyon->mouse_delta_x * sensitivity * state.dt;
  camera.orientation.pitch += tachyon->mouse_delta_y * sensitivity * state.dt;

  camera.rotation = camera.orientation.toQuaternion();
}

void EditorUtilities::SwivelAroundPosition(Tachyon* tachyon, const tVec3f& position) {
  auto& camera = tachyon->scene.camera;
  tVec3f offset = camera.position - position;
  tVec3f unit_offset = offset.unit();

  tCamera3p camera3p;
  camera3p.radius = offset.magnitude();
  camera3p.azimuth = atan2f(unit_offset.z, unit_offset.x);
  camera3p.altitude = atan2f(unit_offset.y, unit_offset.xz().magnitude());

  if (tachyon->mouse_delta_x != 0 || tachyon->mouse_delta_y != 0) {
    camera3p.azimuth += (float)tachyon->mouse_delta_x / 1000.f;
    camera3p.altitude += (float)tachyon->mouse_delta_y / 1000.f;
    camera3p.limitAltitude(0.99f);

    camera.position = position + camera3p.calculatePosition();
  }

  tVec3f facing_direction = position - camera.position;

  camera.orientation.face(facing_direction, tVec3f(0, 1.f, 0));
  camera.rotation = camera.orientation.toQuaternion();
}