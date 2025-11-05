#include <string>

#include "miniaudio/miniaudio.h"

#include "engine/tachyon_sound.h"

#define get_sound(resource) (ma_sound*)resource.data

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
  auto* sound = get_sound(resource);

  ma_sound_seek_to_pcm_frame(sound, 0);
  ma_sound_set_fade_in_milliseconds(sound, -1, volume, 0);
  ma_sound_start(sound);
}

void Tachyon_LoopSound(tSoundResource& resource, const float volume) {
  Tachyon_PlaySound(resource, volume);

  auto* sound = get_sound(resource);

  ma_sound_set_looping(sound, true);
}

void Tachyon_FadeOutSound(tSoundResource& resource, uint64 duration) {
  auto* sound = get_sound(resource);

  ma_sound_set_fade_in_milliseconds(sound, -1, 0, duration);
}

void Tachyon_StopSound(tSoundResource& resource) {
  auto* sound = get_sound(resource);

  ma_sound_stop(sound);
}

void Tachyon_ExitSoundEngine() {
  ma_engine_uninit(&engine);
}