#pragma once

#include "engine/tachyon_aliases.h"
#include "engine/tachyon_types.h"

#define Tachyon_Loop(code)\
  while (Tachyon_IsRunning(tachyon)) {\
    Tachyon_HandleEvents(tachyon);\
    code;\
    Tachyon_RenderScene(tachyon);\
  }\

Tachyon* Tachyon_Init();
void Tachyon_SpawnWindow(Tachyon* tachyon, const char* title, uint32 width, uint32 height);
void Tachyon_UseRenderBackend(Tachyon* tachyon, TachyonRenderBackend backend);
bool Tachyon_IsRunning(Tachyon* tachyon);
void Tachyon_HandleEvents(Tachyon* tachyon);
void Tachyon_RenderScene(Tachyon* tachyon);
void Tachyon_FocusWindow();
void Tachyon_UnfocusWindow();
void Tachyon_Exit(Tachyon* tachyon);