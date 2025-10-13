#include <string>

#include "miniaudio/miniaudio.h"

#include "engine/tachyon_sound.h"

ma_engine engine;

void Tachyon_InitSoundEngine() {
  ma_engine_init(NULL, &engine);
}

tSoundResource Tachyon_CreateSound(const char* file_path) {
  tSoundResource resource;
  resource.data = malloc(sizeof(ma_sound));

  ma_sound_init_from_file(&engine, file_path, 0, NULL, NULL, (ma_sound*)resource.data);

  return resource;
}

void Tachyon_PlaySound(const char* file_path) {
  ma_engine_play_sound(&engine, file_path, NULL);
}

void Tachyon_PlaySound(tSoundResource& resource, const float volume) {
  auto* sound = (ma_sound*)resource.data;

  ma_sound_seek_to_pcm_frame(sound, 0);
  ma_sound_set_fade_in_milliseconds(sound, -1, volume, 0);
  ma_sound_start(sound);
}

void Tachyon_FadeOutSound(tSoundResource& resource) {
  auto* sound = (ma_sound*)resource.data;

  ma_sound_set_fade_in_milliseconds(sound, -1, 0, 1000);
}

void Tachyon_StopSound(tSoundResource& resource) {
  auto* sound = (ma_sound*)resource.data;

  ma_sound_stop(sound);
}

void Tachyon_ExitSoundEngine() {
  ma_engine_uninit(&engine);
}