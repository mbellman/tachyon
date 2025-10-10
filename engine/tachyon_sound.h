#pragma once

#include "engine/tachyon_types.h"

struct tSoundResource {
  void* data = nullptr;
};

void Tachyon_InitSoundEngine();
tSoundResource Tachyon_CreateSound(const char* file_path);
void Tachyon_PlaySound(tSoundResource& sound, const float volume = 1.f);
void Tachyon_FadeOutSound(tSoundResource& sound);
void Tachyon_StopSound(tSoundResource& sound);

void Tachyon_PlaySound(const char* file_path);
void Tachyon_ForcePlaySound(const char* file_path);
void Tachyon_ExitSoundEngine();