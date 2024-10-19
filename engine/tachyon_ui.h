#pragma once

#include "engine/tachyon_types.h"

tUIElement* Tachyon_CreateUIElement(const std::string& path);
void Tachyon_DrawUIElement(Tachyon* tachyon, const tUIElement* ui_element, uint16 x, uint16 y);
void Tachyon_ClearUIDrawCommands(Tachyon* tachyon);