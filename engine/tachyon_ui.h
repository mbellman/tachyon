#pragma once

#include "engine/tachyon_types.h"

tUIElement* Tachyon_CreateUIElement(const std::string& path);
void Tachyon_DrawUIElement(Tachyon* tachyon, const tUIElement* ui_element, int32 x, int32 y, float rotation = 0.f);
void Tachyon_ClearUIDrawCommands(Tachyon* tachyon);