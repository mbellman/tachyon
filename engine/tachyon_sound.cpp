#include <string>

#include "miniaudio/miniaudio.h"

#include "engine/tachyon_sound.h"

ma_engine engine;
std::string current_sound;

void Tachyon_InitSoundEngine() {
  ma_engine_init(NULL, &engine);
}

void Tachyon_PlaySound(const char* file_path) {
  if (current_sound == file_path) {
    return;
  }

  ma_engine_play_sound(&engine, file_path, NULL);

  current_sound = file_path;
}

void Tachyon_ExitSoundEngine() {
  ma_engine_uninit(&engine);
}