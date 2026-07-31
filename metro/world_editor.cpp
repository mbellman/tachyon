#include "metro/world_editor.h"

using namespace metro;

void WorldEditor::Open(Tachyon* tachyon, State& state) {
  state.is_editor_open = true;

  show_overlay_message("Entering editor");
}

void WorldEditor::Update(Tachyon* tachyon, State& state) {

}

void WorldEditor::Close(Tachyon* tachyon, State& state) {
  state.is_editor_open = false;

  show_overlay_message("Leaving editor");
}