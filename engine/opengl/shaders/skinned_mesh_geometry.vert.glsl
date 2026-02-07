#version 460 core

uniform mat4 view_projection_matrix;
uniform mat4 model_matrix;
uniform uint model_surface;
uniform vec3 transform_origin;

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec3 vertexTangent;
layout (location = 3) in vec2 vertexUv;

flat out uvec4 fragSurface;
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

uvec4 SurfaceToUVec4(uint surface) {
  uint rg = ((surface & 0xFF000000) >> 24);
  uint ba = ((surface & 0x00FF0000) >> 16);
  uint roughness_metalness = ((surface & 0x0000FF00) >> 8);
  uint clearcoat_subsurface = surface & 0x000000FF;

  return uvec4(rg, ba, roughness_metalness, clearcoat_subsurface);
}

void main() {
  mat3 normal_matrix = transpose(inverse(mat3(model_matrix)));

  // For the vertex transform, start by just applying rotation + scale
  vec3 model_space_position = mat3(model_matrix) * vertexPosition;

  // Then apply translation, offset by the transform origin
  vec3 translation = vec3(model_matrix[3][0], model_matrix[3][1], model_matrix[3][2]);
  vec3 world_space_position = model_space_position + (translation - transform_origin);

  gl_Position = view_projection_matrix * vec4(world_space_position, 1.0);

  fragSurface = SurfaceToUVec4(model_surface);
  fragNormal = normal_matrix * vertexNormal;
  fragTangent = normal_matrix * vertexTangent;
  fragBitangent = getFragBitangent(fragNormal, fragTangent);
  fragUv = vertexUv;
}