#pragma once

#include "engine/tachyon_aliases.h"
#include "engine/tachyon_types.h"

#define Tachyon_Loop(code)\
  while (Tachyon_IsRunning(tachyon)) {\
    Tachyon_StartFrame(tachyon);\
    float dt = tachyon->last_frame_time_in_microseconds / 1000000.f;\
    code;\
    Tachyon_EndFrame(tachyon);\
  }\

#define is_window_focused() tachyon->is_window_focused
#define add_dev_label(label, message) tachyon->dev_labels.push_back({ label, message })

Tachyon* Tachyon_Init();
void Tachyon_SpawnWindow(Tachyon* tachyon, const char* title, uint32 width, uint32 height);
void Tachyon_UseRenderBackend(Tachyon* tachyon, TachyonRenderBackend backend);
bool Tachyon_IsRunning(Tachyon* tachyon);
void Tachyon_StartFrame(Tachyon* tachyon);
void Tachyon_EndFrame(Tachyon* tachyon);
void Tachyon_FocusWindow(Tachyon* tachyon);
void Tachyon_UnfocusWindow(Tachyon* tachyon);
void Tachyon_Exit(Tachyon* tachyon);