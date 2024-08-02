#pragma once

#include "engine/tachyon_types.h"

Tachyon* Tachyon_Init();
void Tachyon_CreateWindow(Tachyon* tachyon);
void Tachyon_UseRenderBackend(Tachyon* tachyon, TachyonRenderBackend backend);
bool Tachyon_IsRunning(Tachyon* tachyon);
void Tachyon_HandleEvents(Tachyon* tachyon);
void Tachyon_RenderScene(Tachyon* tachyon);
void Tachyon_Exit(Tachyon* tachyon);