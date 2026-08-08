#version 460 core

// uniform sampler2D meshTexture;
uniform mat4 view_projection_matrix;
uniform vec3 transform_origin;

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec3 vertex_normal;
layout (location = 2) in vec3 vertex_tangent;
layout (location = 3) in vec2 vertex_uv;

out vec3 fragWorldPosition;
out vec3 fragNormal;
out vec3 fragTangent;
out vec3 fragBitangent;
out vec2 fragUv;

void main() {
  // Apply translation, offset by the origin
  vec3 world_space_position = vertex_position -transform_origin;

  gl_Position = view_projection_matrix * vec4(world_space_position, 1.0);

  fragWorldPosition = world_space_position;
  fragNormal = vertex_normal;
  fragTangent = vertex_tangent;
  fragUv = vertex_uv;
}