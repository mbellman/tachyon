#pragma once

#include "engine/tachyon_aliases.h"
#include "engine/tachyon_types.h"

#define MAX_DT (1.f / 60.f)

#define Tachyon_RunMainLoop(code)\
  while (tachyon->is_running) {\
    Tachyon_StartFrame(tachyon);\
    float dt = (float)tachyon->last_frame_time_in_microseconds / 1000000.f;\
    if (dt > MAX_DT) dt = MAX_DT;\
    code;\
    Tachyon_EndFrame(tachyon);\
  }\

#define is_window_focused() (tachyon->is_window_focused)
#define add_dev_label(label, message) tachyon->dev_labels.push_back({ label, message })

Tachyon* Tachyon_Init();
void Tachyon_SpawnWindow(Tachyon* tachyon, const char* title, uint32 width, uint32 height);
void Tachyon_UseRenderBackend(Tachyon* tachyon, TachyonRenderBackend backend);
void Tachyon_StartFrame(Tachyon* tachyon);
void Tachyon_EndFrame(Tachyon* tachyon);
void Tachyon_FocusWindow(Tachyon* tachyon);
void Tachyon_UnfocusWindow(Tachyon* tachyon);
void Tachyon_HandleWindowResize(Tachyon* tachyon);
void Tachyon_Exit(Tachyon* tachyon);