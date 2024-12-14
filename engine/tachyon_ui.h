#pragma once

#include "engine/tachyon_types.h"

tUIElement* Tachyon_CreateUIElement(const char* image_path);
tUIText* Tachyon_CreateUIText(const char* font_file_path, const int font_size);
void Tachyon_DrawUIElement(Tachyon* tachyon, const tUIElement* ui_element, const tUIDrawCommandOptions& options);
void Tachyon_DrawUIText(Tachyon* tachyon, const tUIText* ui_text, const tUIDrawCommandOptions& options);
void Tachyon_ClearUIDrawCommands(Tachyon* tachyon);