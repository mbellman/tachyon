#version 460 core

uniform mat4 view_projection_matrix;
uniform vec3 transform_origin;
uniform uint mesh_surface;
// uniform sampler2D texture;

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec3 vertex_normal;
layout (location = 2) in vec3 vertex_tangent;
layout (location = 3) in vec2 vertex_uv;

flat out uvec4 fragSurface;
out vec3 fragWorldPosition;
out vec3 fragNormal;
out vec3 fragTangent;
out vec3 fragBitangent;
out vec2 fragUv;

uvec4 SurfaceToUVec4(uint surface) {
  uint rg = ((surface & 0xFF000000) >> 24);
  uint ba = ((surface & 0x00FF0000) >> 16);
  uint roughness_metalness = ((surface & 0x0000FF00) >> 8);
  uint clearcoat_subsurface = surface & 0x000000FF;

  return uvec4(rg, ba, roughness_metalness, clearcoat_subsurface);
}

void main() {
  // Apply translation, offset by the origin
  vec3 world_space_position = vertex_position -transform_origin;

  gl_Position = view_projection_matrix * vec4(world_space_position, 1.0);

  fragSurface = SurfaceToUVec4(mesh_surface);
  fragWorldPosition = world_space_position;
  fragNormal = vertex_normal;
  fragTangent = vertex_tangent;
  fragUv = vertex_uv;
}