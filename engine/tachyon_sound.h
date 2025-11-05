#pragma once

#include "engine/tachyon_types.h"

struct tSoundResource {
  void* data = nullptr;
};

void Tachyon_InitSoundEngine();
tSoundResource Tachyon_CreateSound(const char* file_path);
void Tachyon_PlaySound(const char* file_path);
void Tachyon_PlaySound(tSoundResource& resource, const float volume = 1.f);
void Tachyon_LoopSound(tSoundResource& resource, const float volume = 1.f);
void Tachyon_FadeOutSound(tSoundResource& resource, uint64 duration);
void Tachyon_StopSound(tSoundResource& resource);
void Tachyon_ExitSoundEngine();