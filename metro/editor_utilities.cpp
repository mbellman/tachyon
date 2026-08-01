#include "metro/editor_utilities.h"
#include "metro/debug.h"

using namespace metro;

void EditorUtilities::ShowPositionGizmo(Tachyon* tachyon, State& state, const tVec3f& position) {
  const float line_thickness = 15.f;
  const tVec3f cone_scale = tVec3f(40.f);

  // Left vector
  {
    DebugLineConfig left_line = {
      .position = position,
      .vector = tVec3f(250.f, 0, 0),
      .color = tVec3f(1.f, 0, 0),
      .thickness = line_thickness
    };

    DebugConeConfig left_cone = {
      .position = left_line.position + left_line.vector,
      .scale = cone_scale,
      .direction = tVec3f(1.f, 0, 0),
      .color = tVec3f(1.f, 0, 0)
    };

    Debug::ShowDebugLine(tachyon, state, left_line);
    Debug::ShowDebugCone(tachyon, state, left_cone);
  }

  // Up vector
  {
    DebugLineConfig up_line = {
      .position = position,
      .vector = tVec3f(0, 250.f, 0),
      .color = tVec3f(0, 1.f, 0),
      .thickness = line_thickness
    };

    DebugConeConfig up_cone = {
      .position = up_line.position + up_line.vector,
      .scale = cone_scale,
      .direction = tVec3f(0, 1.f, 0),
      .color = tVec3f(0, 1.f, 0)
    };

    Debug::ShowDebugLine(tachyon, state, up_line);
    Debug::ShowDebugCone(tachyon, state, up_cone);
  }

  // Forward vector
  {
    DebugLineConfig forward_line = {
      .position = position,
      .vector = tVec3f(0, 0, -250.f),
      .color = tVec3f(0, 0, 1.f),
      .thickness = line_thickness
    };

    DebugConeConfig forward_cone = {
      .position = forward_line.position + forward_line.vector,
      .scale = cone_scale,
      .direction = tVec3f(0, 0, -1.f),
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