#version 460 core

uniform mat4 view_projection_matrix;
uniform vec3 transform_origin;
uniform mat4 model_matrix;
uniform uint model_surface;

uniform mat4 bones[32];

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec3 vertexTangent;
layout (location = 3) in vec2 vertexUv;
layout (location = 4) in uint bone_indexes_packed;
layout (location = 5) in vec4 bone_weights;

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

  uint bone_1_index = (bone_indexes_packed & 0xFF000000) >> 24;
  uint bone_2_index = (bone_indexes_packed & 0x00FF0000) >> 16;
  uint bone_3_index = (bone_indexes_packed & 0x0000FF00) >> 8;
  uint bone_4_index = (bone_indexes_packed & 0x000000FF);

  mat4 bone_1 = bones[bone_1_index];
  mat4 bone_2 = bones[bone_2_index];
  mat4 bone_3 = bones[bone_3_index];
  mat4 bone_4 = bones[bone_4_index];

  // Apply bone transforms
  vec3 position = vec3(0.0);
  vec3 normal = vec3(0.0);

  position += (bone_weights.x * (bone_1 * vec4(vertexPosition, 1.0))).xyz;
  position += (bone_weights.y * (bone_2 * vec4(vertexPosition, 1.0))).xyz;
  position += (bone_weights.z * (bone_3 * vec4(vertexPosition, 1.0))).xyz;
  position += (bone_weights.w * (bone_4 * vec4(vertexPosition, 1.0))).xyz;

  normal += (bone_weights.x * (mat3(bone_1) * vertexNormal)).xyz;
  normal += (bone_weights.y * (mat3(bone_2) * vertexNormal)).xyz;
  normal += (bone_weights.z * (mat3(bone_3) * vertexNormal)).xyz;
  normal += (bone_weights.w * (mat3(bone_4) * vertexNormal)).xyz;

  // For the vertex transform, start by just applying rotation + scale
  vec3 model_space_position = mat3(model_matrix) * position;

  // Then apply translation, offset by the transform origin
  vec3 translation = vec3(model_matrix[3][0], model_matrix[3][1], model_matrix[3][2]);
  vec3 world_space_position = model_space_position + (translation - transform_origin);

  gl_Position = view_projection_matrix * vec4(world_space_position, 1.0);

  fragSurface = SurfaceToUVec4(model_surface);
  fragNormal = normal_matrix * normal;
  fragTangent = normal_matrix * vertexTangent;
  fragBitangent = getFragBitangent(fragNormal, fragTangent);
  fragUv = vertexUv;
}