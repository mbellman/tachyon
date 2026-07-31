#include "metro/world_editor.h"
#include "metro/debug.h"

using namespace metro;

static void ShowPositionGizmo(Tachyon* tachyon, State& state, const tVec3f& position) {
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

void WorldEditor::Open(Tachyon* tachyon, State& state) {
  state.is_editor_open = true;

  show_overlay_message("Entering editor");
}

void WorldEditor::Update(Tachyon* tachyon, State& state) {
  auto& camera = tachyon->scene.camera;

  // @temporary
  tVec3f p = camera.position + camera.orientation.getDirection() * 2000.f;

  ShowPositionGizmo(tachyon, state, p);
}

void WorldEditor::Close(Tachyon* tachyon, State& state) {
  state.is_editor_open = false;

  show_overlay_message("Leaving editor");
}