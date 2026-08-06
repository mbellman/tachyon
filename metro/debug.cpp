#include "engine/tachyon.h"

#include "metro/debug.h"

// @temporary
// @todo move CollisionPlane elsewhere
#include "metro/game_state.h"

using namespace metro;

#define CUBE_MESH(total) Tachyon_AddMesh(tachyon, Tachyon_CreateCubeMesh(), total)
#define SPHERE_MESH(total, divisions) Tachyon_AddMesh(tachyon, Tachyon_CreateSphereMesh(divisions), total)
#define PLANE_MESH(total) Tachyon_AddMesh(tachyon, Tachyon_CreatePlaneMesh(), total)
#define MODEL_MESH(path, total) Tachyon_AddMesh(tachyon, Tachyon_LoadMesh(path), total)

struct Meshes {
  uint16
    debug_line,
    debug_plane,
    debug_cube,
    debug_sphere,
    debug_ring,
    debug_cone,
    debug_box;
} meshes;

struct {
  tUIText* debug_text = nullptr;
} ui;

void Debug::Init(Tachyon* tachyon) {
  // Meshes
  {
    meshes.debug_line   = MODEL_MESH("./metro/3d_models/debug_line.obj", 100);
    meshes.debug_plane  = PLANE_MESH(100);
    meshes.debug_cube   = CUBE_MESH(100);
    meshes.debug_sphere = SPHERE_MESH(100, 12);
    meshes.debug_ring   = MODEL_MESH("./metro/3d_models/debug_ring.obj", 100);
    meshes.debug_cone   = MODEL_MESH("./metro/3d_models/debug_cone.obj", 100);
    meshes.debug_box    = CUBE_MESH(100);

    mesh(meshes.debug_cube).shadow_cascade_ceiling = 0;
    mesh(meshes.debug_sphere).shadow_cascade_ceiling = 0;
    mesh(meshes.debug_ring).shadow_cascade_ceiling = 0;
    mesh(meshes.debug_plane).shadow_cascade_ceiling = 0;
    mesh(meshes.debug_line).shadow_cascade_ceiling = 0;
    mesh(meshes.debug_cone).shadow_cascade_ceiling = 0;

    mesh(meshes.debug_box).type = WIREFRAME_MESH;
  }

  // UI
  {
    ui.debug_text = Tachyon_CreateUIText("./fonts/CascadiaMonoNF.ttf", 16);
  }
}

void Debug::CreateObjects(Tachyon* tachyon) {
  for_range(1, 100) {
    create(meshes.debug_line);
    create(meshes.debug_plane);
    create(meshes.debug_cube);
    create(meshes.debug_sphere);
    create(meshes.debug_ring);
    create(meshes.debug_cone);
    create(meshes.debug_box);
  }
}

void Debug::Reset(Tachyon* tachyon) {
  reset_instances(meshes.debug_line);
  reset_instances(meshes.debug_plane);
  reset_instances(meshes.debug_cube);
  reset_instances(meshes.debug_sphere);
  reset_instances(meshes.debug_ring);
  reset_instances(meshes.debug_cone);
  reset_instances(meshes.debug_box);
}

void Debug::ShowDebugLine(Tachyon* tachyon, const DebugLineConfig& config) {
  auto& line = use_instance(meshes.debug_line);

  const tVec3f& vector = config.vector;
  float length = vector.magnitude();
  tVec3f direction = vector / length;

  tVec3f up = vector.y != 0.f && vector.x == 0.f && vector.z == 0.f
    ? tVec3f(1.f, 0, 0)
    : tVec3f(0, 1.f, 0);

  line.position = config.position;
  line.rotation = Quaternion::FromDirection(direction, up);
  line.scale.x = config.thickness;
  line.scale.y = config.thickness;
  line.scale.z = length;
  line.color = tVec4f(config.color, 0.8f);

  commit(line);
}

void Debug::ShowDebugPlane(Tachyon* tachyon, const CollisionPlane& plane, const tVec3f& color) {
  tVec3f midpoint = (plane.p1 + plane.p2 + plane.p3 + plane.p4) / 4.f;
  float x_scale = tVec3f::distance(plane.p1, plane.p4) / 2.f;
  float z_scale = tVec3f::distance(plane.p1, plane.p2) / 2.f;
  tVec3f forward = (plane.p1 - plane.p2).unit();
  tVec3f up = tVec3f::cross((plane.p3 - plane.p2).unit(), forward);

  auto& debug_plane = use_instance(meshes.debug_plane);

  debug_plane.position = midpoint;
  debug_plane.rotation = Quaternion::FromDirection(forward, up);
  debug_plane.scale = tVec3f(x_scale, 1.f, z_scale);
  debug_plane.color = color;

  commit(debug_plane);
}

void Debug::ShowDebugCube(Tachyon* tachyon, const DebugCubeConfig& config) {
  auto& cube = use_instance(meshes.debug_cube);

  cube.position = config.position;
  cube.rotation = config.rotation;
  cube.scale = config.scale;
  cube.color = tVec4f(config.color, 0.8f);

  commit(cube);
}

void Debug::ShowDebugSphere(Tachyon* tachyon, const tVec3f& position, const float radius) {
  auto& sphere = use_instance(meshes.debug_sphere);

  sphere.position = position;
  sphere.scale = tVec3f(radius);
  sphere.color = 0x00F8;

  commit(sphere);
}

void Debug::ShowDebugRing(Tachyon* tachyon, const DebugShapeConfig& config) {
  auto& ring = use_instance(meshes.debug_ring);

  tVec3f direction = config.direction;

  tVec3f up = direction.y != 0.f && direction.x == 0.f && direction.z == 0.f
    ? tVec3f(1.f, 0, 0)
    : tVec3f(0, 1.f, 0);

  ring.position = config.position;
  ring.scale = config.scale;
  ring.rotation = Quaternion::FromDirection(direction, up);
  ring.color = tVec4f(config.color, 0.8f);

  commit(ring);
}

void Debug::ShowDebugCone(Tachyon* tachyon, const DebugShapeConfig& config) {
  auto& cone = use_instance(meshes.debug_cone);

  tVec3f direction = config.direction;

  tVec3f up = direction.y != 0.f && direction.x == 0.f && direction.z == 0.f
    ? tVec3f(1.f, 0, 0)
    : tVec3f(0, 1.f, 0);

  cone.position = config.position;
  cone.rotation = Quaternion::FromDirection(direction, up);
  cone.scale = config.scale;
  cone.color = tVec4f(config.color, 0.8f);

  commit(cone);
}

void Debug::ShowDebugBox(Tachyon* tachyon, const DebugCubeConfig& config) {
  auto& box = use_instance(meshes.debug_box);

  box.position = config.position;
  box.rotation = config.rotation;
  box.scale = config.scale;
  box.color = tVec4f(config.color, 0.8f);

  commit(box);
}

// @todo refactor to use line + cone
void Debug::ShowDebugVector(Tachyon* tachyon, const tVec3f& position, const tVec3f& vector, const tVec3f& color) {
  float length = vector.magnitude();
  tVec3f direction = vector / length;

  auto& line = use_instance(meshes.debug_line);

  tVec3f up = vector.y != 0.f && vector.x == 0.f && vector.z == 0.f
    ? tVec3f(1.f, 0, 0)
    : tVec3f(0, 1.f, 0);

  line.position = position;
  line.rotation = Quaternion::FromDirection(direction, up);
  line.scale.y = 150.f;
  line.scale.x = 150.f;
  line.scale.z = length;
  line.color = tVec4f(color, 0.8f);

  commit(line);

  auto& cone = use_instance(meshes.debug_cone);

  cone.position = position + vector;
  cone.rotation = line.rotation;
  cone.scale = tVec3f(400.f);
  cone.color = line.color;

  commit(cone);
}

void Debug::ShowDebugLabel(Tachyon* tachyon, const tVec3f& world_position, const tVec2f& offset, const std::string& label) {
  auto& scene = tachyon->scene;
  tCamera camera = scene.camera;

  tMat4f camera_rotation_matrix = camera.rotation.toMatrix4f();

  tMat4f view_matrix = (
    camera_rotation_matrix *
    tMat4f::translation(camera.position * tVec3f(-1.f))
  );

  tMat4f projection_matrix = tMat4f::perspective(camera.fov, scene.z_near, scene.z_far);

  tVec3f local_position = view_matrix * world_position;

  if (local_position.z < -0.1f) {
    tVec3f clip_position = (projection_matrix * local_position) / local_position.z;

    clip_position.x = -0.5f * clip_position.x + 0.5f;
    clip_position.y = 0.5f * clip_position.y + 0.5f;

    Tachyon_DrawUIText(tachyon, ui.debug_text, {
      .screen_x = (int32) (clip_position.x * tachyon->window_width + offset.x),
      .screen_y = (int32) (clip_position.y * tachyon->window_height + offset.y),
      .centered = false,
      .background = tVec4f(0, 0, 0.2f, 0.5f),
      .string = label
    });
  }
}