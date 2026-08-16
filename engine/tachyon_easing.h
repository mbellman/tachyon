#pragma once

float Tachyon_Lerpf(const float a, const float b, const float alpha);
float Tachyon_InverseLerp(const float a, const float b, const float alpha);
float Tachyon_LerpCircularf(float a, float b, float alpha, float max_range);
float Tachyon_EaseOutSine(float t);
float Tachyon_EaseOutQuad(float t);
float Tachyon_EaseInOutf(float t);
float Tachyon_EaseInOutSinef(float t);
float Tachyon_EaseOutBackf(float t);