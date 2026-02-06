#pragma once

#include <string>

#include "engine/tachyon_linear_algebra.h"

struct Quaternion {
  float w = 0.f;
  float x = 0.f;
  float y = 0.f;
  float z = 0.f;

  Quaternion() {};
  Quaternion(float f): w(f), x(f), y(f), z(f) {};
  Quaternion(float w, float x, float y, float z): w(w), x(x), y(y), z(z) {};

  static Quaternion fromAxisAngle(float angle, float x, float y, float z);
  static Quaternion fromAxisAngle(const tVec3f& axis, float angle);
  static Quaternion FromDirection(const tVec3f& forward, const tVec3f& up);
  static Quaternion fromEulerAngles(float x, float y, float z);
  static Quaternion nlerp(const Quaternion& q1, const Quaternion& q2, float alpha);
  static Quaternion slerp(const Quaternion& q1, const Quaternion& q2, float alpha);

  bool operator==(const Quaternion& q2) const;
  Quaternion operator*(const Quaternion& q2) const;
  void operator*=(const Quaternion& q2);

  // @todo rename getForwardDirection()
  tVec3f getDirection() const;
  tVec3f getLeftDirection() const;
  tVec3f getUpDirection() const;
  tMat4f toMatrix4f() const;
  Quaternion opposite() const;
  Quaternion unit() const;

  std::string toString() const;
};