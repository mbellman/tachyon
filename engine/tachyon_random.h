#pragma once

#include <random>

struct tRNG {
  tRNG();
  tRNG(float seed);

  float Random();
  float Random(float low, float high);
  int RandomInt(int low, int high);

  std::default_random_engine engine;
};

float Tachyon_GetRandom();
float Tachyon_GetRandom(float low, float high);
int Tachyon_GetRandom(int low, int high);