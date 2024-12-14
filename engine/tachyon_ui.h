#pragma once

#include "engine/tachyon_types.h"

tUIElement* Tachyon_CreateUIElement(const char* image_path);
tUIText* Tachyon_CreateUIText(const char* font_file_path, const int font_size);
void Tachyon_DrawUIElement(Tachyon* tachyon, const tUIElement* ui_element, int32 x, int32 y, float rotation = 0.f, float alpha = 1.f);
void Tachyon_DrawUIText(Tachyon* tachyon, const tUIText* ui_text, int32 x, int32 y, float rotation = 0.f, float alpha = 1.f);
void Tachyon_ClearUIDrawCommands(Tachyon* tachyon);