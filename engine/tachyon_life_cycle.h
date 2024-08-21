#pragma once

#include "engine/tachyon_aliases.h"
#include "engine/tachyon_types.h"

#define Tachyon_Loop(code)\
  while (Tachyon_IsRunning(tachyon)) {\
    Tachyon_StartFrame(tachyon);\
    code;\
    Tachyon_EndFrame(tachyon);\
  }\

Tachyon* Tachyon_Init();
void Tachyon_SpawnWindow(Tachyon* tachyon, const char* title, uint32 width, uint32 height);
void Tachyon_UseRenderBackend(Tachyon* tachyon, TachyonRenderBackend backend);
bool Tachyon_IsRunning(Tachyon* tachyon);
void Tachyon_StartFrame(Tachyon* tachyon);
void Tachyon_EndFrame(Tachyon* tachyon);
void Tachyon_FocusWindow(Tachyon* tachyon);
void Tachyon_UnfocusWindow(Tachyon* tachyon);
void Tachyon_Exit(Tachyon* tachyon);