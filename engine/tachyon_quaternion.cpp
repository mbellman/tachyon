#include <format>
#include <math.h>

#include "engine/tachyon_quaternion.h"

Quaternion Quaternion::fromAxisAngle(float angle, float x, float y, float z) {
  float sa = sinf(angle / 2.0f);

  return {
    cosf(angle / 2.0f),
    x * sa,
    y * sa,
    z * sa
  };
}

Quaternion Quaternion::fromAxisAngle(const tVec3f& axis, float angle) {
  float sa = sinf(angle / 2.0f);

  return {
    cosf(angle / 2.0f),
    axis.x * sa,
    axis.y * sa,
    axis.z * sa
  };
}

/**
 * Adapted from https://forum.playcanvas.com/t/quaternion-from-direction-vector/6369/3
 */
Quaternion Quaternion::FromDirection(const tVec3f& forward, const tVec3f& up) {
  auto vector = forward;
  auto vector2 = tVec3f::cross(up, vector).unit();
  auto vector3 = tVec3f::cross(vector, vector2);
  auto m00 = vector2.x;
  auto m01 = vector2.y;
  auto m02 = vector2.z;
  auto m10 = vector3.x;
  auto m11 = vector3.y;
  auto m12 = vector3.z;
  auto m20 = vector.x;
  auto m21 = vector.y;
  auto m22 = vector.z;

  auto num8 = (m00 + m11) + m22;
  Quaternion quaternion;
  if (num8 > 0.f)
  {
      auto num = sqrtf(num8 + 1.f);
      quaternion.w = num * 0.5f;
      num = 0.5f / num;
      quaternion.x = (m12 - m21) * num;
      quaternion.y = (m20 - m02) * num;
      quaternion.z = (m01 - m10) * num;
      return quaternion;
  }
  if ((m00 >= m11) && (m00 >= m22))
  {
      auto num7 = sqrtf(((1.f + m00) - m11) - m22);
      auto num4 = 0.5f / num7;
      quaternion.x = 0.5f * num7;
      quaternion.y = (m01 + m10) * num4;
      quaternion.z = (m02 + m20) * num4;
      quaternion.w = (m12 - m21) * num4;
      return quaternion;
  }
  if (m11 > m22)
  {
      auto num6 = sqrtf(((1.f + m11) - m00) - m22);
      auto num3 = 0.5f / num6;
      quaternion.x = (m10 + m01) * num3;
      quaternion.y = 0.5f * num6;
      quaternion.z = (m21 + m12) * num3;
      quaternion.w = (m20 - m02) * num3;
      return quaternion;
  }
  auto num5 = sqrtf(((1.f + m22) - m00) - m11);
  auto num2 = 0.5f / num5;
  quaternion.x = (m20 + m02) * num2;
  quaternion.y = (m21 + m12) * num2;
  quaternion.z = 0.5f * num5;
  quaternion.w = (m01 - m10) * num2;
  return quaternion;
}

Quaternion Quaternion::fromEulerAngles(float x, float y, float z) {
  Quaternion pitch = Quaternion::fromAxisAngle(x, 1.0f, 0.0f, 0.0f);
  Quaternion yaw = Quaternion::fromAxisAngle(y, 0.0f, 1.0f, 0.0f);
  Quaternion roll = Quaternion::fromAxisAngle(z, 0.0f, 0.0f, 1.0f);

  return roll * pitch * yaw;
}

/**
 * @source: https://wrf.ecse.rpi.edu/wiki/ComputerGraphicsFall2013/guha/Code/quaternionAnimation.cpp
 */
Quaternion Quaternion::slerp(const Quaternion& q1, const Quaternion& q2, float alpha) {
  float w1, x1, y1, z1, w2, x2, y2, z2, w3, x3, y3, z3;
  float theta, mult1, mult2;

  w1 = q1.w;
  x1 = q1.x;
  y1 = q1.y;
  z1 = q1.z; 

  w2 = q2.w;
  x2 = q2.x;
  y2 = q2.y;
  z2 = q2.z;

  if (w1*w2 + x1*x2 + y1*y2 + z1*z2 < 0.f) {
    w2 = -w2;
    x2 = -x2;
    y2 = -y2;
    z2 = -z2;
  }

  theta = acosf(w1*w2 + x1*x2 + y1*y2 + z1*z2);

  if (theta > 0.000001f) {
    mult1 = sinf( (1.f-alpha)*theta ) / sinf( theta );
    mult2 = sinf( alpha*theta ) / sinf( theta );
  } else {
    mult1 = 1.f - alpha;
    mult2 = alpha;
  }

  w3 =  mult1*w1 + mult2*w2;
  x3 =  mult1*x1 + mult2*x2;
  y3 =  mult1*y1 + mult2*y2;
  z3 =  mult1*z1 + mult2*z2;

  Quaternion r;

  r.w = w3;
  r.x = x3;
  r.y = y3;
  r.z = z3;

  return r.unit();
}

bool Quaternion::operator==(const Quaternion& q2) const {
  return (
    q2.w == w &&
    q2.x == x &&
    q2.y == y &&
    q2.z == z
  );
}

Quaternion Quaternion::operator*(const Quaternion& q2) const {
  return {
    w * q2.w - x * q2.x - y * q2.y - z * q2.z,
    w * q2.x + x * q2.w + y * q2.z - z * q2.y,
    w * q2.y - x * q2.z + y * q2.w + z * q2.x,
    w * q2.z + x * q2.y - y * q2.x + z * q2.w
  };
}

void Quaternion::operator*=(const Quaternion& q2) {
  *this = q2 * *this;
}

tVec3f Quaternion::getDirection() const {
  const static tVec3f forward = tVec3f(0, 0, -1.f);

  return toMatrix4f().transformVec3f(forward);
}

tVec3f Quaternion::getUpDirection() const {
  const static tVec3f up = tVec3f(0, 1.f, 0);

  return toMatrix4f().transformVec3f(up);
}

tVec3f Quaternion::getLeftDirection() const {
  const static tVec3f left = tVec3f(-1.f, 0, 0);

  return toMatrix4f().transformVec3f(left);
}

tMat4f Quaternion::toMatrix4f() const {
  return {
    1 - 2 * y * y - 2 * z * z, 2 * x * y - 2 * z * w, 2 * x * z + 2 * y * w, 0.f,
    2 * x * y + 2 * z * w, 1 - 2 * x * x - 2 * z * z, 2 * y * z - 2 * x * w, 0.f,
    2 * x * z - 2 * y * w, 2 * y * z + 2 * x * w, 1 - 2 * x * x - 2 * y * y, 0.f,
    0.f, 0.f, 0.f, 1.f
  };
}

Quaternion Quaternion::opposite() const {
  return Quaternion(w, -x, -y, -z);
}

Quaternion Quaternion::unit() const {
  auto magnitude = sqrtf(w*w + x*x + y*y + z*z);

  return {
    w / magnitude,
    x / magnitude,
    y / magnitude,
    z / magnitude
  };
}

std::string Quaternion::toString() const {
  return std::format("w: {:.3f}, x: {:.3f}, y: {:.3f}, z: {:.3f}", w, x, y, z);
}