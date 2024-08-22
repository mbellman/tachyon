#include <math.h>

#include "engine/tachyon_aliases.h"
#include "engine/tachyon_linear_algebra.h"
#include "engine/tachyon_quaternion.h"

void tVec3f::operator+=(const tVec3f& v) {
  x += v.x;
  y += v.y;
  z += v.z;
}

tVec3f tVec3f::operator*(const tVec3f& v) const {
  return {
    x * v.x,
    y * v.y,
    z * v.z
  };
}

void tVec3f::operator*=(const tVec3f& v) {
  x *= v.x;
  y *= v.y;
  z *= v.z;
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

tMat4f tMat4f::perspective(float fov, float near, float far) {
  constexpr float DEGRESS_TO_RADIANS = 3.141592f / 180.f;
  constexpr float aspectRatio = 1920.f / 1080.f;

  // float aspectRatio = (float)area.width / (float)area.height;
  float f = 1.0f / tanf(fov / 2.f * DEGRESS_TO_RADIANS);
  float nf = near - far;

  return {
    f / aspectRatio, 0.0f, 0.0f, 0.0f,
    0.0f, f, 0.0f, 0.0f,
    0.0f, 0.0f, (far + near) / nf, (2 * far * near) / nf,
    0.0f, 0.0f, -1.0f, 0.0f
  };
}

tMat4f tMat4f::scale(const tVec3f& scale) {
  return {
    scale.x, 0.0f, 0.0f, 0.0f,
    0.0f, scale.y, 0.0f, 0.0f,
    0.0f, 0.0f, scale.z, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
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
  m_transform.m[15] = 1.0f;

  return m_transform;
}

tMat4f tMat4f::translation(const tVec3f& translation) {
  return {
    1.0f, 0.0f, 0.0f, translation.x,
    0.0f, 1.0f, 0.0f, translation.y,
    0.0f, 0.0f, 1.0f, translation.z,
    0.0f, 0.0f, 0.0f, 1.0f
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