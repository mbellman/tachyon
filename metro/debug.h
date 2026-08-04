#pragma once

#include "engine/tachyon_types.h"

namespace metro {
  struct CollisionPlane;

  struct DebugLineConfig {
    tVec3f position;
    tVec3f vector;
    tVec3f color;
    float thickness;
  };

  struct DebugShapeConfig {
    tVec3f position;
    tVec3f scale;
    tVec3f direction;
    tVec3f color;
  };

  struct DebugCubeConfig {
    tVec3f position;
    tVec3f scale;
    Quaternion rotation;
    tVec3f color;
  };

  namespace Debug {
    void Init(Tachyon* tachyon);
    void CreateObjects(Tachyon* tachyon);
    void Reset(Tachyon* tachyon);

    void ShowDebugLine(Tachyon* tachyon, const DebugLineConfig& config);
    void ShowDebugPlane(Tachyon* tachyon, const CollisionPlane& plane, const tVec3f& color);
    void ShowDebugCube(Tachyon* tachyon, const DebugCubeConfig& config);
    void ShowDebugSphere(Tachyon* tachyon, const tVec3f& position, const float radius);
    void ShowDebugRing(Tachyon* tachyon, const DebugShapeConfig& config);
    void ShowDebugCone(Tachyon* tachyon, const DebugShapeConfig& config);
    void ShowDebugBox(Tachyon* tachyon, const DebugCubeConfig& config);
    void ShowDebugVector(Tachyon* tachyon, const tVec3f& position, const tVec3f& vector, const tVec3f& color);

    void ShowDebugLabel(Tachyon* tachyon, const tVec3f& world_position, const tVec2f& offset, const std::string& label);
  }
}