#include <SDL_image.h>

#include "engine/tachyon_ui.h"

// @todo Tachyon_DestroyUIElement(tUIElement* element)
tUIElement* Tachyon_CreateUIElement(const char* image_path) {
  auto* element = new tUIElement;

  element->surface = IMG_Load(image_path);

  return element;
}

tUIText* Tachyon_CreateUIText(const char* font_file_path, const int font_size) {
  auto* text = new tUIText;

  text->font = TTF_OpenFont(font_file_path, font_size);

  return text;
}

void Tachyon_DrawUIElement(Tachyon* tachyon, const tUIElement* ui_element, const tUIDrawCommandOptions& options) {
  tUIDrawCommand command = {
    .ui_element = ui_element,
    .options = options
  };

  tachyon->ui_draw_commands.push_back(command);
}

void Tachyon_DrawUIText(Tachyon* tachyon, const tUIText* ui_text, const tUIDrawCommandOptions& options) {
  tUIDrawCommand command = {
    .ui_text = ui_text,
    .options = options
  };

  tachyon->ui_draw_commands.push_back(command);
}

void Tachyon_ClearUIDrawCommands(Tachyon* tachyon) {
  tachyon->ui_draw_commands.clear();
}