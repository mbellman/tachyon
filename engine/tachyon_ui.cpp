#include <SDL_image.h>

#include "engine/tachyon_ui.h"

tUIElement* Tachyon_CreateUIElement(const std::string& file) {
  auto* element = new tUIElement;

  element->surface = IMG_Load(file.c_str());

  return element;
}

void Tachyon_DrawUIElement(Tachyon* tachyon, const tUIElement* ui_element, int32 x, int32 y, float rotation, float alpha) {
  tUIDrawCommand command = {
    .ui_element = ui_element,
    .screen_x = x,
    .screen_y = y,
    .rotation = rotation,
    .alpha = alpha
  };

  tachyon->ui_draw_commands.push_back(command);
}

void Tachyon_ClearUIDrawCommands(Tachyon* tachyon) {
  tachyon->ui_draw_commands.clear();
}