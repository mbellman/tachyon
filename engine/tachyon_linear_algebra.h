#pragma once

struct Quaternion;

struct tVec2f {
  float x = 0.f;
  float y = 0.f;
};

struct tVec3f {
  float x = 0.f;
  float y = 0.f;
  float z = 0.f;
};

struct tVec4f {
  float x = 0.f;
  float y = 0.f;
  float z = 0.f;
  float w = 0.f;
};

struct tMat4f {
  float m[16] = { 0.f };

  static tMat4f perspective(float fov, float near, float far);
  static tMat4f scale(const tVec3f& scale);
  static tMat4f transformation(const tVec3f& translation, const tVec3f& scale, const Quaternion& rotation);

  tVec3f transformVec3f(const tVec3f& vector) const;
  tMat4f transpose() const;
};