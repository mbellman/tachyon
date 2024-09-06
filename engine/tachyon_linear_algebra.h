#pragma once

#include <string>

struct Quaternion;

struct tVec2f {
  float x = 0.f;
  float y = 0.f;
};

struct tVec3f {
  float x = 0.f;
  float y = 0.f;
  float z = 0.f;

  tVec3f() {};
  tVec3f(float f) : x(f), y(f), z(f) {};
  tVec3f(float x, float y, float z) : x(x), y(y), z(z) {};

  tVec3f operator+(const tVec3f& v) const;
  tVec3f operator-(const tVec3f& v) const;
  tVec3f operator*(const tVec3f& v) const;
  void operator+=(const tVec3f& v);
  void operator*=(const tVec3f& v);

  static tVec3f cross(const tVec3f& v1, const tVec3f& v2);
  static float dot(const tVec3f& v1, const tVec3f& v2);

  tVec3f invert() const;
  float magnitude() const;
  tVec3f unit() const;
  tVec3f xz() const;

  void debug() const;
  std::string toString() const;
};

struct tVec4f {
  float x = 0.f;
  float y = 0.f;
  float z = 0.f;
  float w = 0.f;

  tVec4f() {};
  tVec4f(float f) : x(f), y(f), z(f), w(f) {};
  tVec4f(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {};
};

struct tMat4f {
  float m[16] = { 0.f };

  tMat4f operator*(const tMat4f& matrix) const;

  static tMat4f perspective(float fov, float near, float far);
  static tMat4f scale(const tVec3f& scale);
  static tMat4f transformation(const tVec3f& translation, const tVec3f& scale, const Quaternion& rotation);
  static tMat4f translation(const tVec3f& position);

  tMat4f inverse() const;
  tVec3f transformVec3f(const tVec3f& vector) const;
  tMat4f transpose() const;
};