#include "metro/editor_utilities.h"
#include "metro/debug.h"

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

void EditorUtilities::ShowPositionGizmo(Tachyon* tachyon, State& state, const tVec3f& position, const Quaternion& basis_rotation) {
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

    DebugConeConfig left_cone = {
      .position = left_line.position + left_line.vector,
      .scale = cone_scale,
      .direction = vector,
      .color = tVec3f(1.f, 0, 0)
    };

    Debug::ShowDebugLine(tachyon, state, left_line);
    Debug::ShowDebugCone(tachyon, state, left_cone);
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

    DebugConeConfig up_cone = {
      .position = up_line.position + up_line.vector,
      .scale = cone_scale,
      .direction = vector,
      .color = tVec3f(0, 1.f, 0)
    };

    Debug::ShowDebugLine(tachyon, state, up_line);
    Debug::ShowDebugCone(tachyon, state, up_cone);
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

    DebugConeConfig forward_cone = {
      .position = forward_line.position + forward_line.vector,
      .scale = cone_scale,
      .direction = vector,
      .color = tVec3f(0, 0, 1.f)
    };

    Debug::ShowDebugLine(tachyon, state, forward_line);
    Debug::ShowDebugCone(tachyon, state, forward_cone);
  }
}

void EditorUtilities::SwivelAroundPosition(Tachyon* tachyon, State& state, const tVec3f& position) {
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

  tVec3f direction = position - camera.position;

  camera.orientation.face(direction, tVec3f(0, 1.f, 0));
  camera.rotation = camera.orientation.toQuaternion();
}