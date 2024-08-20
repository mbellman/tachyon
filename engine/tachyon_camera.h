#pragma once

#include "engine/tachyon_quaternion.h"
#include "engine/tachyon_linear_algebra.h"

struct tOrientation {
  float roll = 0.0f;
  float pitch = 0.0f;
  float yaw = 0.0f;

  tOrientation() {};
  // @todo pitch, yaw, roll
  tOrientation(float roll, float pitch, float yaw): roll(roll), pitch(pitch), yaw(yaw) {};

  void operator+=(const tOrientation& orientation);

  tVec3f getDirection() const;
  tVec3f getLeftDirection() const;
  tVec3f getRightDirection() const;
  tVec3f getUpDirection() const;
  void face(const tVec3f& forward, const tVec3f& up);
  tOrientation invert() const;
  Quaternion toQuaternion() const;
  tVec3f toVec3f() const;
};

struct tCamera {
  tVec3f position;
  tOrientation orientation;
  float fov = 45.f;
  Quaternion rotation = tOrientation(0, 0, 0).toQuaternion();
};

struct tCamera3p {
  float azimuth = 0.0f;
  float altitude = 0.0f;
  float radius = 100.0f;

  tVec3f calculatePosition() const;
  bool isUpsideDown() const;
  void limitAltitude(float factor);
};