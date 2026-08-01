#include "metro/world_editor.h"
#include "metro/editor_utilities.h"

using namespace metro;

void WorldEditor::Open(Tachyon* tachyon, State& state) {
  state.is_editor_open = true;

  show_overlay_message("Entering editor");
}

void WorldEditor::Update(Tachyon* tachyon, State& state) {
  auto& camera = tachyon->scene.camera;

  // @temporary
  tVec3f p = camera.position + camera.orientation.getDirection() * 2000.f;

  EditorUtilities::ShowPositionGizmo(tachyon, state, p);
}

void WorldEditor::Close(Tachyon* tachyon, State& state) {
  state.is_editor_open = false;

  show_overlay_message("Leaving editor");
}