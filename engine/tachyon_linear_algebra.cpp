#include <format>
#include <math.h>
#include <stdio.h>

#include "engine/tachyon_aliases.h"
#include "engine/tachyon_constants.h"
#include "engine/tachyon_linear_algebra.h"
#include "engine/tachyon_quaternion.h"

tVec3f tVec3f::operator+(const tVec3f& v) const {
  return {
    x + v.x,
    y + v.y,
    z + v.z
  };
}

tVec3f tVec3f::operator-(const tVec3f& v) const {
  return {
    x - v.x,
    y - v.y,
    z - v.z
  };
}

tVec3f tVec3f::operator*(const tVec3f& v) const {
  return {
    x * v.x,
    y * v.y,
    z * v.z
  };
}

tVec3f tVec3f::operator/(const float f) const {
  return {
    x / f,
    y / f,
    z / f
  };
}

bool tVec3f::operator==(const tVec3f& v) const {
  return x == v.x && y == v.y && z == v.z;
}

void tVec3f::operator+=(const tVec3f& v) {
  x += v.x;
  y += v.y;
  z += v.z;
}

void tVec3f::operator-=(const tVec3f& v) {
  x -= v.x;
  y -= v.y;
  z -= v.z;
}

void tVec3f::operator*=(const tVec3f& v) {
  x *= v.x;
  y *= v.y;
  z *= v.z;
}

void tVec3f::operator/=(const float f) {
  x /= f;
  y /= f;
  z /= f;
}

tVec3f tVec3f::cross(const tVec3f& v1, const tVec3f& v2) {
  return {
    v1.y * v2.z - v1.z * v2.y,
    v1.z * v2.x - v1.x * v2.z,
    v1.x * v2.y - v1.y * v2.x
  };
}

float tVec3f::dot(const tVec3f& v1, const tVec3f& v2) {
  return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

tVec3f tVec3f::invert() const {
  return *this * -1.f;
}

float tVec3f::magnitude() const {
  return sqrtf(x*x + y*y + z*z);
}

tVec3f tVec3f::unit() const {
  float m = magnitude();

  return {
    x / m,
    y / m,
    z / m
  };
}

tVec3f tVec3f::xz() const {
  return {
    x,
    0.f,
    z
  };
}

void tVec3f::debug() const {
  printf("{ x: %f, y: %f, z: %f }\n", x, y, z);
}

std::string tVec3f::toString() const {
  return std::format("x: {:.3f}, y: {:.3f}, z: {:.3f}", x, y, z);
}

tVec4f tVec4f::operator*(const tVec4f& v) const {
  return {
    x * v.x,
    y * v.y,
    z * v.z,
    w * v.w
  };
}

tVec3f tVec4f::homogenize() const {
  return {
    x / w,
    y / w,
    z / w
  };
}

tMat4f tMat4f::operator*(const tMat4f& matrix) const {
  tMat4f product;

  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      float& value = product.m[r * 4 + c] = 0;

      for (int n = 0; n < 4; n++) {
        value += m[r * 4 + n] * matrix.m[n * 4 + c];
      }
    }
  }

  return product;
}

tVec3f tMat4f::operator*(const tVec3f& vector) const {
  return transformVec3f(vector);
}

tVec4f tMat4f::operator*(const tVec4f& vector) const {
  float x = vector.x;
  float y = vector.y;
  float z = vector.z;
  float w = vector.w;

  return tVec4f(
    x * m[0] + y * m[1] + z * m[2] + w * m[3],
    x * m[4] + y * m[5] + z * m[6] + w * m[7],
    x * m[8] + y * m[9] + z * m[10] + w * m[11],
    x * m[12] + y * m[13] + z * m[14] + w * m[15]
  );
}

tMat4f tMat4f::inverse() const {
  float A2323 = m[10] * m[15] - m[11] * m[14];
  float A1323 = m[9] * m[15] - m[11] * m[13];
  float A1223 = m[9] * m[14] - m[10] * m[13];
  float A0323 = m[8] * m[15] - m[11] * m[12];
  float A0223 = m[8] * m[14] - m[10] * m[12];
  float A0123 = m[8] * m[13] - m[9] * m[12];
  float A2313 = m[6] * m[15] - m[7] * m[14];
  float A1313 = m[5] * m[15] - m[7] * m[13];
  float A1213 = m[5] * m[14] - m[6] * m[13];
  float A2312 = m[6] * m[11] - m[7] * m[10];
  float A1312 = m[5] * m[11] - m[7] * m[9];
  float A1212 = m[5] * m[10] - m[6] * m[9];
  float A0313 = m[4] * m[15] - m[7] * m[12];
  float A0213 = m[4] * m[14] - m[6] * m[12];
  float A0312 = m[4] * m[11] - m[7] * m[8];
  float A0212 = m[4] * m[10] - m[6] * m[8];
  float A0113 = m[4] * m[13] - m[5] * m[12];
  float A0112 = m[4] * m[9] - m[5] * m[8];

  float determinant = 1.f / (
    m[0] * (m[5] * A2323 - m[6] * A1323 + m[7] * A1223) -
    m[1] * (m[4] * A2323 - m[6] * A0323 + m[7] * A0223) +
    m[2] * (m[4] * A1323 - m[5] * A0323 + m[7] * A0123) -
    m[3] * (m[4] * A1223 - m[5] * A0223 + m[6] * A0123)
  );

  tMat4f inverse;

  inverse.m[0] = determinant *  (m[5] * A2323 - m[6] * A1323 + m[7] * A1223);
  inverse.m[1] = determinant * -(m[1] * A2323 - m[2] * A1323 + m[3] * A1223);
  inverse.m[2] = determinant *  (m[1] * A2313 - m[2] * A1313 + m[3] * A1213);
  inverse.m[3] = determinant * -(m[1] * A2312 - m[2] * A1312 + m[3] * A1212);
  inverse.m[4] = determinant * -(m[4] * A2323 - m[6] * A0323 + m[7] * A0223);
  inverse.m[5] = determinant *  (m[0] * A2323 - m[2] * A0323 + m[3] * A0223);
  inverse.m[6] = determinant * -(m[0] * A2313 - m[2] * A0313 + m[3] * A0213);
  inverse.m[7] = determinant *  (m[0] * A2312 - m[2] * A0312 + m[3] * A0212);
  inverse.m[8] = determinant *  (m[4] * A1323 - m[5] * A0323 + m[7] * A0123);
  inverse.m[9] = determinant * -(m[0] * A1323 - m[1] * A0323 + m[3] * A0123);
  inverse.m[10] = determinant *  (m[0] * A1313 - m[1] * A0313 + m[3] * A0113);
  inverse.m[11] = determinant * -(m[0] * A1312 - m[1] * A0312 + m[3] * A0112);
  inverse.m[12] = determinant * -(m[4] * A1223 - m[5] * A0223 + m[6] * A0123);
  inverse.m[13] = determinant *  (m[0] * A1223 - m[1] * A0223 + m[2] * A0123);
  inverse.m[14] = determinant * -(m[0] * A1213 - m[1] * A0213 + m[2] * A0113);
  inverse.m[15] = determinant *  (m[0] * A1212 - m[1] * A0212 + m[2] * A0112);

  return inverse;
}

tMat4f tMat4f::perspective(float fov, float near, float far) {
  constexpr float DEGRESS_TO_RADIANS = t_PI / 180.f;
  // @todo pass width/height as arguments
  constexpr float aspectRatio = 1920.f / 1080.f;

  float f = 1.f / tanf(fov / 2.f * DEGRESS_TO_RADIANS);
  float nf = near - far;

  return {
    f / aspectRatio, 0.f, 0.f, 0.f,
    0.f, f, 0.f, 0.f,
    0.f, 0.f, (far + near) / nf, (2 * far * near) / nf,
    0.f, 0.f, -1.f, 0.f
  };
}

tMat4f tMat4f::orthographic(float top, float bottom, float left, float right, float near, float far) {
  return {
    2.f / (right - left), 0.f, 0.f, -(right + left) / (right - left),
    0.f, 2.f / (top - bottom), 0.f, -(top + bottom) / (top - bottom),
    0.f, 0.f, -2.f / (far - near), -(far + near) / (far - near),
    0.f, 0.f, 0.f, 1.f
  };
}

tMat4f tMat4f::lookAt(const tVec3f& eye, const tVec3f& direction, const tVec3f& top) {
  tVec3f forward = direction.unit();
  tVec3f right = tVec3f::cross(top, forward).unit();
  tVec3f up = tVec3f::cross(forward, right).unit();
  tMat4f translation = tMat4f::translation(eye.invert());

  tMat4f rotation = {
    right.x, right.y, right.z, 0.0f,
    up.x, up.y, up.z, 0.0f,
    forward.x, forward.y, forward.z, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
  };

  return rotation * translation;
}

tMat4f tMat4f::scale(const tVec3f& scale) {
  return {
    scale.x, 0.f, 0.f, 0.f,
    0.f, scale.y, 0.f, 0.f,
    0.f, 0.f, scale.z, 0.f,
    0.f, 0.f, 0.f, 1.f
  };
}

tMat4f tMat4f::transformation(const tVec3f& translation, const tVec3f& scale, const Quaternion& rotation) {
  tMat4f m_transform;
  tMat4f m_scale = tMat4f::scale(scale);
  tMat4f m_rotation = rotation.toMatrix4f();

  // Declares a small float buffer which helps reduce the number
  // of cache misses in the scale * rotation loop. Scale terms
  // can be written in sequentially, followed by rotation terms,
  // followed by a sequential read when multiplying the buffered
  // terms. Confers a ~5-10% speedup, which is appreciable once
  // the number of transforms per frame reaches into the thousands.
  float v[6];

  // Accumulate rotation * scale
  for (uint32 r = 0; r < 3; r++) {
    // Store rotation terms
    v[0] = m_rotation.m[r * 4];
    v[2] = m_rotation.m[r * 4 + 1];
    v[4] = m_rotation.m[r * 4 + 2];

    for (uint32 c = 0; c < 3; c++) {
      // Store scale terms
      v[1] = m_scale.m[c];
      v[3] = m_scale.m[4 + c];
      v[5] = m_scale.m[8 + c];

      // rotation * scale
      m_transform.m[r * 4 + c] = (
        v[0] * v[1] +
        v[2] * v[3] +
        v[4] * v[5]
      );
    }
  }

  // Apply translation directly
  m_transform.m[3] = translation.x;
  m_transform.m[7] = translation.y;
  m_transform.m[11] = translation.z;
  m_transform.m[15] = 1.f;

  return m_transform;
}

tMat4f tMat4f::translation(const tVec3f& translation) {
  return {
    1.f, 0.f, 0.f, translation.x,
    0.f, 1.f, 0.f, translation.y,
    0.f, 0.f, 1.f, translation.z,
    0.f, 0.f, 0.f, 1.f
  };
}

tVec3f tMat4f::transformVec3f(const tVec3f& vector) const {
  float x = vector.x;
  float y = vector.y;
  float z = vector.z;

  return tVec3f(
    x * m[0] + y * m[1] + z * m[2] + m[3],
    x * m[4] + y * m[5] + z * m[6] + m[7],
    x * m[8] + y * m[9] + z * m[10] + m[11]
  );
}

tMat4f tMat4f::transpose() const {
  return {
    m[0], m[4], m[8], m[12],
    m[1], m[5], m[9], m[13],
    m[2], m[6], m[10], m[14],
    m[3], m[7], m[11], m[15]
  };
}