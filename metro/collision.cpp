#include "metro/collision.h"

using namespace metro;

#define min(a, b) (a > b ? b : a)
#define max(a, b) (a > b ? a : b)

const static std::vector<tVec3f> FACE_PLANE_POINTS = {
  tVec3f(-1.f, 1.f, -1.f ),
  tVec3f(-1.f, 1.f, 1.f),
  tVec3f(1.f, 1.f, 1.f),
  tVec3f(1.f, 1.f, -1.f)
};

const static std::vector<tVec3f> SLOPE_PLANE_POINTS = {
  tVec3f(-1.f, -1.f, -1.f ),
  tVec3f(-1.f, 1.f, 1.f),
  tVec3f(1.f, 1.f, 1.f),
  tVec3f(1.f, -1.f, -1.f)
};

static inline bool IsInBetween(float n, float a, float b) {
  return n >= min(a, b) && n <= max(a, b);
}

void Collision::PrepareCollisionPlane(CollisionPlane& plane) {
  plane.normal = tVec3f::cross(plane.p2 - plane.p1, plane.p3 - plane.p2).unit();

  plane.t1 = tVec3f::cross(plane.normal, plane.p2 - plane.p1);
  plane.t2 = tVec3f::cross(plane.normal, plane.p3 - plane.p2);
  plane.t3 = tVec3f::cross(plane.normal, plane.p4 - plane.p3);
  plane.t4 = tVec3f::cross(plane.normal, plane.p1 - plane.p4);
}

CollisionPlane Collision::CreateFloorCollisionPlane(const Transform& transform) {
  tMat4f rotation = transform.rotation.toMatrix4f();
  auto& scale = transform.scale;

  CollisionPlane plane;

  auto& p1 = FACE_PLANE_POINTS[0];
  auto& p2 = FACE_PLANE_POINTS[1];
  auto& p3 = FACE_PLANE_POINTS[2];
  auto& p4 = FACE_PLANE_POINTS[3];

  plane.p1 = transform.position + (rotation * (scale * p1));
  plane.p2 = transform.position + (rotation * (scale * p2));
  plane.p3 = transform.position + (rotation * (scale * p3));
  plane.p4 = transform.position + (rotation * (scale * p4));

  PrepareCollisionPlane(plane);

  return plane;
}

CollisionPlane Collision::CreateSlopeCollisionPlane(const Transform& transform) {
  tMat4f rotation = transform.rotation.toMatrix4f();
  auto& scale = transform.scale;

  CollisionPlane plane;

  auto& p1 = SLOPE_PLANE_POINTS[0];
  auto& p2 = SLOPE_PLANE_POINTS[1];
  auto& p3 = SLOPE_PLANE_POINTS[2];
  auto& p4 = SLOPE_PLANE_POINTS[3];

  plane.p1 = transform.position + (rotation * (scale * p1));
  plane.p2 = transform.position + (rotation * (scale * p2));
  plane.p3 = transform.position + (rotation * (scale * p3));
  plane.p4 = transform.position + (rotation * (scale * p4));

  PrepareCollisionPlane(plane);

  return plane;
}

CollisionTest Collision::TestRayHit(tVec3f& ray_start, tVec3f& ray, CollisionPlane& plane) {
  CollisionTest test;

  tVec3f ray_end = ray_start + ray;
  tVec3f line = ray_end - ray_start;

  if (tVec3f::dot(plane.normal, line) == 0.f) {
    return test;
  }

  float n_dot_p = tVec3f::dot(plane.normal, plane.p1);
  float length = (n_dot_p - tVec3f::dot(plane.normal, ray_start)) / tVec3f::dot(plane.normal, line);
  tVec3f point = ray_start + line * length;
  tVec3f end = ray_start + line;

  if (
    // If the point is on the line segment...
    IsInBetween(point.x, ray_start.x, end.x) &&
    IsInBetween(point.y, ray_start.y, end.y) &&
    IsInBetween(point.z, ray_start.z, end.z) &&
    // And the point is inside the plane area...
    tVec3f::dot(point - plane.p1, plane.t1) >= 0.f &&
    tVec3f::dot(point - plane.p2, plane.t2) >= 0.f &&
    tVec3f::dot(point - plane.p3, plane.t3) >= 0.f &&
    tVec3f::dot(point - plane.p4, plane.t4) >= 0.f
  ) {
    test.collision_point = point;
    test.has_collision = true;
  }

  return test;
}