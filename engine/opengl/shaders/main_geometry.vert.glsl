#version 460 core

// uniform sampler2D meshTexture;
uniform mat4 mat_view_projection;

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec3 vertexTangent;
layout (location = 3) in vec2 vertexUv;
layout (location = 4) in uint modelSurface;
layout (location = 5) in mat4 modelMatrix;

flat out vec3 fragColor;
flat out vec4 fragMaterial;
out vec3 fragPosition;
out vec3 fragNormal;
out vec3 fragTangent;
out vec3 fragBitangent;
out vec2 fragUv;

/**
 * Returns a bitangent from potentially non-orthonormal
 * normal/tangent vectors using the Gram-Schmidt process.
 */
vec3 getFragBitangent(vec3 normal, vec3 tangent) {
  // Redefine the tangent by using the projection of the tangent
  // onto the normal line and defining a vector from that to the
  // original tangent, orthonormalizing the normal/tangent
  tangent = normalize(tangent - dot(tangent, normal) * normal);

  return cross(tangent, normal);
}

vec3 UnpackColor(uint surface) {
  float r = float((surface & 0xF0000000) >> 28) / 15.0;
  float g = float((surface & 0x0F000000) >> 24) / 15.0;
  float b = float((surface & 0x00F00000) >> 20) / 15.0;

  return vec3(r, g, b);
}

vec4 UnpackMaterial(uint surface) {
  float r = float((surface & 0x0000F000) >> 12) / 15.0;
  float m = float((surface & 0x00000F00) >> 8) / 15.0;
  float c = float((surface & 0x000000F0) >> 4) / 15.0;
  float s = float(surface & 0x0000000F) / 15.0;

  return vec4(r, m, c, s);
}

void main() {
  vec4 world_position = modelMatrix * vec4(vertexPosition, 1.0);
  mat3 normal_matrix = transpose(inverse(mat3(modelMatrix)));

  gl_Position = mat_view_projection * world_position;

  fragColor = UnpackColor(modelSurface);
  fragMaterial = UnpackMaterial(modelSurface);
  fragPosition = world_position.xyz;
  fragNormal = normal_matrix * vertexNormal;
  fragTangent = normal_matrix * vertexTangent;
  fragBitangent = getFragBitangent(fragNormal, fragTangent);

  fragUv = vertexUv;
}